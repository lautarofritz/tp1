#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "protocol.h"

#define INITIAL_HEADER_BYTES 16
#define INITIAL_SIGNATURE_BYTES 7

static int _parse_header(protocol_t *message, byte *message_buffer);

//escribe el parámetro en el header
static void _write_header(protocol_t *self, byte *string, int space_count);
//
static void _header_assign(protocol_t *self, uint32_t old_header_len, int space_count);

static int _padding_space(int n);

static void _parse_body(protocol_t *self, byte *message_buffer, int i);

static void _signature_initialize(protocol_t *self);

static void _signature_update(protocol_t *self);

static void _signature_padding(protocol_t *self, int param_count);

static void _write_body(protocol_t *self, byte *string);

static int _assemble_message(protocol_t *self, byte **message_buffer);

void protocol_initialize(protocol_t *self, uint32_t message_id){
	message_id++;
	uint32_t id = htonl(message_id);
	uint32_t header_len = INITIAL_HEADER_BYTES;
	self -> header = malloc(INITIAL_HEADER_BYTES);
	*(self -> header) = 'l';
	*(self -> header + 1) = 1;
	*(self -> header + 2) = 0;
	*(self -> header + 3) = 1;
	memcpy((self -> header + 8), &id, sizeof(uint32_t));
	self -> header_len = INITIAL_HEADER_BYTES;
	self -> signature_len = 0;
	self -> signature_padding = 0;
	self -> body_len = 0;
	self -> signature = NULL;
	self -> body = NULL;
}

int protocol_translate(protocol_t *self, byte **message_buffer){
	int i = _parse_header(self, *message_buffer);
	_parse_body(self, *message_buffer, i);
	int signature_padding = _assemble_message(self, message_buffer);
	return signature_padding;
}

static int _parse_header(protocol_t *self, byte *message_buffer){
	byte *string = calloc(1, sizeof(byte));
	int i = 0, j = 0, space_count = 0;

	while(message_buffer[i] != '('){
		if(message_buffer[i] == ' '){
			string = realloc(string, i - j + 1);
			strncpy(string, message_buffer + j, i - j);
			string[i - j] = '\0';
			_write_header(self, string, space_count);
			space_count++;
			j = i + 1;
		}
		i++;
	}

	string = realloc(string, i - j + 1);
	strncpy(string, message_buffer + j, i - j);
	string[i - j] = '\0';
	_write_header(self, string, space_count);
	free(string);
	i++;
	return i;
}

static void _write_header(protocol_t *self, byte *string, int space_count){ 
	uint32_t new_header_len = self -> header_len + 8 + _padding_space(strlen(string));
	self -> header = realloc(self -> header, new_header_len);
	_header_assign(self, self -> header_len, space_count);
	uint32_t string_size = htonl(strlen(string));
	memcpy((self -> header + self -> header_len + 4), &string_size, sizeof(uint32_t));
	strncpy((self -> header + self -> header_len + 8), string, strlen(string));
	for(int i = self -> header_len + 8 + strlen(string); i < new_header_len; i++)
		*(self -> header + i) = '\0';
	self -> header_len = new_header_len;
}

static void _header_assign(protocol_t *self, uint32_t old_header_len, int space_count){
	byte *aux = (self -> header + old_header_len);
	*(aux + 1) = 1;
	*(aux + 3) = '\0';
	switch(space_count){
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

static void _parse_body(protocol_t *self, byte *message_buffer, int i){
	byte *string = calloc(1, sizeof(byte));
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
	self -> signature = malloc(INITIAL_SIGNATURE_BYTES);
	*(self -> signature) = 8;
	*(self -> signature + 1) = 1;
	*(self -> signature + 2) = 'g';
	*(self -> signature + 3) = '\0';
	*(self -> signature + 4) = 1;
	*(self -> signature + 5) = 's';
	*(self -> signature + 6) = '\0';
	self -> signature_len = INITIAL_SIGNATURE_BYTES;
	self -> body = malloc(sizeof(byte));
}

static void _signature_update(protocol_t *self){
	int signature_len = *(self -> signature + 4);
	signature_len++;
	*(self -> signature + 4) = signature_len;
	self -> signature = realloc(self -> signature, self -> signature_len + 1);
	*(self -> signature + self -> signature_len - 1) = 's';
	*(self -> signature + self -> signature_len) = '\0';
	self -> signature_len++;
}

static void _signature_padding(protocol_t *self, int param_count){
	if(((param_count + 6) % 8) != 0){													
		self -> signature_padding = 8 - ((param_count + 6) % 8);
		self -> signature = realloc(self -> signature, self -> signature_len + self -> signature_padding);	
	}
	for(int i = 0; i < self -> signature_padding; i++)
		*(self -> signature + self -> signature_len + i) = '\0';
}

static void _write_body(protocol_t *self, byte *string){
	uint32_t string_len = htonl(strlen(string));
	self -> body = realloc(self -> body, self -> body_len + strlen(string) + 5);
	memcpy((self -> body + self -> body_len), &string_len, sizeof(uint32_t));
	strncpy(self -> body + self -> body_len + 4, string, strlen(string));
	*(self -> body + self -> body_len + 4 + strlen(string)) = '\0';
	self -> body_len += strlen(string) + 5;
}

static int _assemble_message(protocol_t *self, byte **message_buffer){
	byte *aux_buffer = calloc(1, sizeof(byte));
	uint32_t body_len = htonl(self -> body_len);
	uint32_t full_header_len = self -> header_len + self -> signature_len - INITIAL_HEADER_BYTES;
	full_header_len = htonl(full_header_len);
	memcpy((self -> header + 12), &full_header_len, sizeof(uint32_t));
	memcpy((self -> header + 4), &body_len, sizeof(uint32_t));
	uint32_t full_signature_len = self -> signature_len + self -> signature_padding;
	uint32_t total_len = self -> header_len + self -> body_len + full_signature_len;
	aux_buffer = realloc(aux_buffer, total_len);
	memcpy(aux_buffer, self -> header, self -> header_len);
	memcpy(aux_buffer + self -> header_len, self -> signature, full_signature_len);
	memcpy(aux_buffer + self -> header_len + full_signature_len, self -> body, self -> body_len);
	free(*message_buffer);
	*message_buffer = aux_buffer;
	return self -> signature_padding;
}

void protocol_destroy(protocol_t *self){
	if(self -> signature != NULL && self -> body != NULL){
		free(self -> body);
		free(self -> signature);
	}
	free(self -> header);
}
