#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "common_protocol.h"

static int _parse_header(protocol_t *message, char *message_buffer);

//escribe el parámetro en el header
static void _write_header(protocol_t *self, char *string, int spaces);
//
static void _header_assign(protocol_t *self, uint32_t header_len, int spaces);

static int _padding_space(int n);

static void _parse_body(protocol_t *self, char *message_buffer, int i);

static void _signature_initialize(protocol_t *self);

static void _signature_update(protocol_t *self);

static void _signature_padding(protocol_t *self, int param_count);

static void _write_body(protocol_t *self, char *string);

static int _assemble_message(protocol_t *self, char **message_buffer);

void protocol_initialize(protocol_t *self, uint32_t message_id){
	message_id++;
	uint32_t id = htonl(message_id);
	self -> header = malloc(INITIAL_HEADER_BYTES);
	*(self -> header) = 'l';
	*(self -> header + 1) = 1;
	*(self -> header + 2) = 0;
	*(self -> header + 3) = 1;
	memcpy((self -> header + 8), &id, sizeof(uint32_t));
	self -> header_len = INITIAL_HEADER_BYTES;
	self -> sign_len = 0;
	self -> sign_padding = 0;
	self -> body_len = 0;
	self -> sign = NULL;
	self -> body = NULL;
}

int protocol_translate(protocol_t *self, char **message_buffer){
	int i = _parse_header(self, *message_buffer);
	_parse_body(self, *message_buffer, i);
	int sign_padding = _assemble_message(self, message_buffer);
	return sign_padding;
}

static int _parse_header(protocol_t *self, char *message_buffer){
	char *string = calloc(1, sizeof(char));
	int i = 0, j = 0, spaces = 0;

	while(message_buffer[i] != '('){
		if(message_buffer[i] == ' '){
			string = realloc(string, i - j + 1);
			strncpy(string, message_buffer + j, i - j);
			string[i - j] = '\0';
			_write_header(self, string, spaces);
			spaces++;
			j = i + 1;
		}
		i++;
	}

	string = realloc(string, i - j + 1);
	strncpy(string, message_buffer + j, i - j);
	string[i - j] = '\0';
	_write_header(self, string, spaces);
	free(string);
	i++;
	return i;
}

static void _write_header(protocol_t *self, char *string, int spaces){ 
	uint32_t new_header_len;
	new_header_len = self -> header_len + 8 + _padding_space(strlen(string));
	self -> header = realloc(self -> header, new_header_len);
	_header_assign(self, self -> header_len, spaces);
	uint32_t str_len = htonl(strlen(string));
	memcpy((self->header + self->header_len + 4), &str_len, sizeof(uint32_t));
	strncpy((self -> header + self -> header_len + 8), string, strlen(string));
	for(int i = self->header_len + 8 + strlen(string); i < new_header_len; i++)
		*(self -> header + i) = '\0';
	self -> header_len = new_header_len;
}

static void _header_assign(protocol_t *self, uint32_t header_len, int spaces){
	char *aux = (self -> header + header_len);
	*(aux + 1) = 1;
	*(aux + 3) = '\0';
	switch(spaces){
		case 0:
			*(aux) = 6;
			*(aux + 2) = 's';
		break;
		case 1:
			*(aux) = 1;
			*(aux + 2) = 'o';
		break;
		case 2:
			*(aux) = 2;
			*(aux + 2) = 's';
		break;
		case 3:
			*(aux) = 3;
			*(aux + 2) = 's';
		break;
	}
}

static int _padding_space(int n){
	if(((n + 1) % 8) == 0)
		return n + 1;
	return n * sizeof(char) + 1 + (8 - ((n + 1) % 8));
}

static void _parse_body(protocol_t *self, char *message_buffer, int i){
	char *string = calloc(1, sizeof(char));
	int k = i, j = i, param_count = 0;
	bool signature_init = false;

	while(message_buffer[k] != ')'){
		if(!signature_init){
			param_count++;
			_signature_initialize(self);
			signature_init = true;
		}
		if(message_buffer[k] == ','){
			string = realloc(string, k - j + 1);
			strncpy(string, message_buffer + j, k - j);
			string[k - j] = '\0';
			param_count++;
			_write_body(self, string);
			_signature_update(self);
			j = k + 1;
		}
		k++;
	}

	if(signature_init){
		string = realloc(string, k - j + 1);
		strncpy(string, message_buffer + j, k - j);
		string[k - j] = '\0';
		_write_body(self, string);
		_signature_padding(self, param_count);
		j = k + 1;
	}
	free(string);
}

static void _signature_initialize(protocol_t *self){
	self -> sign = malloc(INITIAL_SIGNATURE_BYTES);
	*(self -> sign) = 8;
	*(self -> sign + 1) = 1;
	*(self -> sign + 2) = 'g';
	*(self -> sign + 3) = '\0';
	*(self -> sign + 4) = 1;
	*(self -> sign + 5) = 's';
	*(self -> sign + 6) = '\0';
	self -> sign_len = INITIAL_SIGNATURE_BYTES;
	self -> body = malloc(sizeof(char));
}

static void _signature_update(protocol_t *self){
	int sign_len = *(self -> sign + 4);
	sign_len++;
	*(self -> sign + 4) = sign_len;
	self -> sign = realloc(self -> sign, self -> sign_len + 1);
	*(self -> sign + self -> sign_len - 1) = 's';
	*(self -> sign + self -> sign_len) = '\0';
	self -> sign_len++;
}

static void _signature_padding(protocol_t *self, int param_count){
	if(((param_count + 6) % 8) != 0){													
		self -> sign_padding = 8 - ((param_count + 6) % 8);
		self->sign = realloc(self->sign, self->sign_len + self->sign_padding);
	}
	for(int i = 0; i < self -> sign_padding; i++)
		*(self -> sign + self -> sign_len + i) = '\0';
}

static void _write_body(protocol_t *self, char *string){
	uint32_t string_len = htonl(strlen(string));
	self->body = realloc(self->body, self->body_len + strlen(string) + 5);
	memcpy((self -> body + self -> body_len), &string_len, sizeof(uint32_t));
	strncpy(self -> body + self -> body_len + 4, string, strlen(string));
	*(self -> body + self -> body_len + 4 + strlen(string)) = '\0';
	self -> body_len += strlen(string) + 5;
}

static int _assemble_message(protocol_t *self, char **message_buffer){
	char *aux_buf = calloc(1, sizeof(char));
	uint32_t body_len = htonl(self -> body_len);
	uint32_t header_len;
	header_len = self->header_len + self->sign_len - INITIAL_HEADER_BYTES;
	header_len = htonl(header_len);
	memcpy((self -> header + 12), &header_len, sizeof(uint32_t));
	memcpy((self -> header + 4), &body_len, sizeof(uint32_t));
	uint32_t sign_len = self -> sign_len + self -> sign_padding;
	uint32_t total_len = self -> header_len + self -> body_len + sign_len;
	aux_buf = realloc(aux_buf, total_len);
	memcpy(aux_buf, self->header, self->header_len);
	memcpy(aux_buf + self->header_len, self->sign, sign_len);
	memcpy(aux_buf + self->header_len + sign_len, self->body, self->body_len);
	free(*message_buffer);
	*message_buffer = aux_buf;
	return self -> sign_padding;
}

void protocol_destroy(protocol_t *self){
	if(self -> sign != NULL && self -> body != NULL){
		free(self -> body);
		free(self -> sign);
	}
	free(self -> header);
}
