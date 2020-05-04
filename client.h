#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "common_socket.h"

#define INITIAL_FD -1
#define BUFFER_LEN 32
#define INITIAL_HEADER_BYTES 16
#define RESPONSE_LEN 3

typedef struct {
	socket_t socket;
	uint32_t sent_messages;
}client_t;

int client_initialize(client_t *self, const char* hostname, const char* port);

int client_execute(client_t *self, FILE* input);

int client_destroy(client_t *self);

#endif
