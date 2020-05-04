#ifndef SERVER_H
#define SERVER_H

#include "common_socket.h"

#define INITIAL_FD -1
#define INITIAL_HEADER_BYTES 16
#define RESPONSE_LEN 3

typedef struct {
	socket_t socket;
} server_t;

int server_initialize(server_t *self, const char* hostname, const char* port);

int server_execute(server_t *self);

int server_destroy(server_t *self);

#endif
