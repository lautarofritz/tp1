#define _POSIX_C_SOURCE 201112L
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "common_socket.h"

static list *_get_addr_list(const char* hostname, const char* port, int flag);

void socket_initialize(socket_t *self, int fd){
    self -> fd = fd;
}

int socket_connect(socket_t *self, const char* hostname, const char* port){
    struct addrinfo *result_list = _get_addr_list(hostname, port, 0);
    struct addrinfo *ptr;
    int fd;
    for (ptr = result_list; ptr != NULL; ptr = ptr -> ai_next) {
        fd = socket(ptr -> ai_family, ptr -> ai_socktype, ptr -> ai_protocol);
        if (fd == -1)
            continue;
        if (connect(fd, ptr -> ai_addr, ptr -> ai_addrlen) != -1)
            break;                  
        close(fd);
    }

    if (ptr == NULL)
        return -1;
    self -> fd = fd;
    freeaddrinfo(result_list);
    return 0;
}

int socket_bind_listen(socket_t *self, const char* hostname, const char* port){
    struct addrinfo *result_list = _get_addr_list(hostname, port, AI_PASSIVE);
    struct addrinfo *ptr;
    int fd;
    for (ptr = result_list; ptr != NULL; ptr = ptr -> ai_next){
        fd = socket(ptr -> ai_family, ptr -> ai_socktype, ptr -> ai_protocol);
        if (fd == -1)
            continue;

        if (bind(fd, ptr -> ai_addr, ptr -> ai_addrlen) == 0)
            break;
        close(fd);
    }
    
    if (ptr == NULL) 
        return -1;
    self -> fd = fd;
    freeaddrinfo(result_list);
    if (listen(self -> fd, 1) == -1)
        return -1;
    return 0;
}

static list* _get_addr_list(const char* hostname, const char* port, int flag){
	struct addrinfo hints, *result_list; 

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;      
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = flag;
    
    getaddrinfo(hostname, port, &hints, &result_list);
    
    return result_list;
}

int socket_accept(socket_t *self){
    int accept_fd = accept(self -> fd, NULL, NULL);
    return accept_fd;
}

int socket_send(socket_t *self, char *msg, int len, int offset){
    int o = offset; //para cumplir con los 80 caracteres por línea
    int total = 0, sent = 0;
    while (total < len){
       sent = send(self -> fd, &msg[total + o], len - total, MSG_NOSIGNAL);
       if (sent == -1)
           return 1;
       total += sent;
    }
    return 0;
}

int socket_receive(socket_t *self, char buf[], int len){
    int total_recv = 0, received = 0;
    while (total_recv < len){
       received = recv(self -> fd, &buf[total_recv], len - total_recv, 0);
       if (received == -1)
           return 1;
       if (received == 0)
           return 0;
       total_recv += received;
    }
    return 2;
}

int socket_destroy(socket_t *self){
    if (shutdown(self -> fd, SHUT_RDWR) == -1)
        return 1;
    if (close(self -> fd) == -1)
        return 1;
    return 0;
}
