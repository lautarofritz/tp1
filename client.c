#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include "client.h"
#include "common_protocol.h"

//toma una línea del archivo para su posterior traducción
//si no alcanzo el final del archivo, retorna false
//caso contrario, retorna true
static bool _get_msg(FILE *input, char file_buf[], char **msg_buf, int *offset);

//le otorga el mensaje traducido al socket para su envío
//primero manda los 16 bytes iniciales, y luego el header y el body (si lo hay)
//luego, recibe la respuesta del servidor
//retorna 0 si el envío fue exitoso o 1 si hubo algún problema durante este
static int _send_message(client_t *self, char *buf, int sign_padding);

int client_initialize(client_t *self, const char* hostname, const char* port){
	socket_t socket;
	socket_initialize(&socket, INITIAL_FD);
	self -> socket = socket;
	if(socket_connect(&self -> socket, hostname, port) == -1)
		return 1;
	self -> sent_messages = 0;
	return 0;
}

int client_execute(client_t *self, FILE* input){
	protocol_t protocol;
	char *message_buffer = calloc(1, sizeof(char));
	char file_buffer[BUFFER_LEN + 1];
	int sign_padding, offset = 0, status;
	int *ptr = &offset;
	bool file_end;

	do{
		protocol_initialize(&protocol, self -> sent_messages);
		memset(message_buffer, '\0', strlen(message_buffer));
		file_end = _get_msg(input, file_buffer, &message_buffer, ptr);
		sign_padding = protocol_translate(&protocol, &message_buffer);
		status = _send_message(self, message_buffer, sign_padding);
		if(status == 1){
			protocol_destroy(&protocol);
			return 1;
		}
		protocol_destroy(&protocol);
	}while(!file_end);

	free(message_buffer);
	return 0;
}

int client_destroy(client_t *self){
	if(socket_destroy(&self -> socket) == 1)
		return 1;
	return 0;
}

static bool _get_msg(FILE *input, char file_buf[], char **msg_buf, int *offset){
	bool line_end = false, file_end = false;
	int i;
	char *aux_buffer = calloc(1, sizeof(char));

	while(!line_end){
		if(fread(file_buf, sizeof(char), BUFFER_LEN, input) != BUFFER_LEN)
			file_end = true;
		file_buf[BUFFER_LEN] = '\0';
		for(i = 0; i < BUFFER_LEN; i++){
			if(file_buf[i] == '\n'){
				line_end = true;
				break;
			}
		}
		aux_buffer = realloc(aux_buffer, strlen(aux_buffer) + i + 1);
		strncat(aux_buffer, file_buf, i);
		aux_buffer[strlen(aux_buffer)] = '\0';
	}

	*offset += strlen(aux_buffer) + 1;
	free(*msg_buf);
	*msg_buf = aux_buffer;
	fseek(input, *offset, SEEK_SET);
	return file_end;
}

static int _send_message(client_t *self, char *buf, int sign_padding){
	int offset = 0;
	uint32_t header_len, body_len;
	memcpy(&body_len, &buf[4], sizeof(int));
    memcpy(&header_len, &buf[12], sizeof(int));
    body_len = ntohl(body_len);
    header_len = ntohl(header_len);
	char response[RESPONSE_LEN];

	if(socket_send(&self -> socket, buf, INITIAL_HEADER_BYTES, offset) == 1)				
		return 1;
	offset = INITIAL_HEADER_BYTES;
	if(socket_send(&self -> socket, buf, header_len, offset) == 1)
		return 1;
	offset += header_len + sign_padding;
	if(socket_send(&self -> socket, buf, body_len, offset) == 1)
		return 1;
	self -> sent_messages++;
	socket_receive(&self -> socket, response, RESPONSE_LEN);
	printf("0x%08x: %s", self -> sent_messages, response);
	return 0;
}
