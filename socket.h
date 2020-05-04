#ifndef SOCKET_H
#define SOCKET_H

typedef struct{
	int fd;
}socket_t;

int socket_initialize(socket_t *self, int fd);

static struct addrinfo* _get_address_list(int flag, const char* hostname, const char* port);

int socket_connect(socket_t *self, int flag, const char* hostname, const char* port);

int socket_bind_listen(socket_t *self, int flag, const char* hostname, const char* port);

int socket_accept(socket_t *self);

int socket_send(socket_t *self, unsigned char *msg, int len, int offset);

int socket_receive(socket_t *self, unsigned char buf[], int len);

int socket_destroy(socket_t *self);

#endif
