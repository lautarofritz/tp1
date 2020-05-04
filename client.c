#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include "client.h"
#include "protocol.h"

#define FLAG 0
#define INITIAL_FD -1
#define BUFFER_LEN 32
#define INITIAL_HEADER_BYTES 16
#define RESPONSE_LEN 3

int client_initialize(client_t *self, const char* hostname, const char* port){
	socket_t socket;
	self -> socket = socket;
	socket_initialize(&self -> socket, INITIAL_FD);
	if(socket_connect(&self -> socket, FLAG, hostname, port) == -1)
		return 1;
	self -> sent_messages = 0;
	return 0;
}

int client_execute(client_t *self, FILE* input){
	protocol_t protocol;
	unsigned char *message_buffer = calloc(1, sizeof(unsigned char));
	unsigned char file_buffer[BUFFER_LEN + 1];
	int sign_padding, offset = 0, status;
	int *ptr = &offset;
	bool file_end;

	do{
		protocol_initialize(&protocol, self -> sent_messages);
		memset(message_buffer, '\0', strlen(message_buffer));
		file_end = _get_message(input, file_buffer, &message_buffer, ptr);
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

static bool _get_message(FILE *input, unsigned char file_buf[], unsigned char **msg_buf, int *offset){
	bool line_end = false, file_end = false;
	int i;
	unsigned char *aux_buffer = calloc(1, sizeof(unsigned char));

	while(!line_end){
		if(fread(file_buf, sizeof(unsigned char), BUFFER_LEN, input) != BUFFER_LEN)
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

static int _send_message(client_t *self, unsigned char *buf, int sign_padding){
	int offset = 0;
	uint32_t header_len, body_len;
	memcpy(&body_len, &buf[4], sizeof(int));
    memcpy(&header_len, &buf[12], sizeof(int));
    body_len = ntohl(body_len);
    header_len = ntohl(header_len);
	unsigned char response[RESPONSE_LEN];

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