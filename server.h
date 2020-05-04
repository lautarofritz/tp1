#ifndef SERVER_H
#define SERVER_H

#include "socket.h"

typedef struct {
	socket_t socket;
} server_t;

int server_initialize(server_t *self, const char* hostname, const char* port);

int server_execute(server_t *self);

int server_destroy(server_t *self);

#endif
