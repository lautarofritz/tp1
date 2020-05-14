#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "common_protocol.h"

//lee el mensaje hasta encontrar un espacio, en cuyo caso llama
//a la función _write_header
//retorna la posición del primer caracter del primer parámetro
//(o del caracter ')' si no hay ninguno)
static int _parse_header(protocol_t *message, char *message_buffer);

static void _write_header(protocol_t *self, char *str, int spaces);

//asigna valores correspondientes a cada parámetro según su tipo
//y los escribe en el header
static void _header_assign(protocol_t *self, uint32_t header_len, int spaces);

static int _padding_space(int n);

//lee cada parámetro hasta encontrar una coma, en cuyo caso llama
//a la función _write_body
static void _parse_body(protocol_t *self, char *message_buffer, int i);

//asigna ciertos valores comunes a todas las firmas
//y los escribe en el buffer de firma
static void _signature_initialize(protocol_t *self);

static void _signature_update(protocol_t *self);

static void _signature_padding(protocol_t *self, int param_count);

static void _write_body(protocol_t *self, char *str);

//junta las tres "partes" del mensaje (header, signature y body)
//y escribe el mensaje entero en el buffer provisto por el client
//retorna el número de bytes de padding de la firma
static int _assemble_message(protocol_t *self, char **message_buffer);

void protocol_initialize(protocol_t *self, uint32_t message_id){
	message_id++;
	uint32_t id = message_id;
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
	char *str;
	int i = 0, j = 0, spaces = 0;

	while(message_buffer[i] != '('){
		if(message_buffer[i] == ' '){
			str = malloc(i - j + 1);
			strncpy(str, message_buffer + j, i - j);
			str[i - j] = '\0';
			_write_header(self, str, spaces);
			spaces++;
			j = i + 1;
			free(str);
		}
		i++;
	}

	str = malloc(i - j + 1);
	strncpy(str, message_buffer + j, i - j);
	str[i - j] = '\0';
	_write_header(self, str, spaces);
	free(str);
	i++;
	return i;
}

static void _write_header(protocol_t *self, char *str, int spaces){ 
	uint32_t new_header_len;
	new_header_len = self -> header_len + 8 + _padding_space(strlen(str));
	self -> header = realloc(self -> header, new_header_len);
	_header_assign(self, self -> header_len, spaces);
	uint32_t str_len = strlen(str);
	memcpy((self->header + self->header_len + 4), &str_len, sizeof(uint32_t));
	strncpy((self -> header + self -> header_len + 8), str, strlen(str));
	for(int i = self->header_len + 8 + strlen(str); i < new_header_len; i++)
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
	char *str;
	int k = i, j = i, param_count = 0;
	bool signature_init = false;

	while(message_buffer[k] != ')'){
		if(!signature_init){
			param_count++;
			_signature_initialize(self);
			signature_init = true;
		}
		if(message_buffer[k] == ','){
			str = malloc(k -j + 1);
			strncpy(str, message_buffer + j, k - j);
			str[k - j] = '\0';
			param_count++;
			_write_body(self, str);
			_signature_update(self);
			j = k + 1;
			free(str);
		}
		k++;
	}

	if(signature_init){
		str = malloc(k - j + 1);
		strncpy(str, message_buffer + j, k - j);
		str[k - j] = '\0';
		_write_body(self, str);
		_signature_padding(self, param_count);
		j = k + 1;
		free(str);
	}
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

static void _write_body(protocol_t *self, char *str){
	uint32_t str_len = strlen(str);
	self->body = realloc(self->body, self->body_len + strlen(str) + 5);
	memcpy((self -> body + self -> body_len), &str_len, sizeof(uint32_t));
	strncpy(self -> body + self -> body_len + 4, str, strlen(str));
	*(self -> body + self -> body_len + 4 + strlen(str)) = '\0';
	self -> body_len += strlen(str) + 5;
}

static int _assemble_message(protocol_t *self, char **message_buffer){
	char *aux_buf;
	uint32_t body_len = self -> body_len;
	uint32_t header_len;
	header_len = self->header_len + self->sign_len - INITIAL_HEADER_BYTES;
	memcpy((self -> header + 12), &header_len, sizeof(uint32_t));
	memcpy((self -> header + 4), &body_len, sizeof(uint32_t));
	uint32_t sign_len = self -> sign_len + self -> sign_padding;
	uint32_t total_len = self -> header_len + self -> body_len + sign_len;
	aux_buf = malloc(total_len);
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
