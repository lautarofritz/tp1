#ifndef SERVER_H
#define SERVER_H

#include "socket.h"

typedef struct {
	socket_t socket;
} server_t;

int server_initialize(server_t *self, const char* hostname, const char* port);

int _server_accept(server_t *self, socket_t *peer_socket);

int server_execute(server_t *self);

int server_destroy(server_t *self);

static int _recv_message(socket_t *peer_socket);

static void _print_message(unsigned char initial_buf[], unsigned char header_buf[], int header_len);

static void _print_header(unsigned char param_type, unsigned char *param_value);

static void _print_body(unsigned char body_buf[], int body_len);

#endif
