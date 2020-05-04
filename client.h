#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "socket.h"

typedef struct {
	socket_t socket;
	uint32_t sent_messages;
}client_t;

int client_initialize(client_t *self, const char* hostname, const char* port);

int client_execute(client_t *self, FILE* input);

int client_destroy(client_t *self);

#endif
