#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include "server.h"
#include "common_socket.h"

static int _server_accept(server_t *self, socket_t *peer_socket);

static int _recv_message(socket_t *peer_socket);

static void _print_message(char initial_buf[], char header_buf[], int header_len);

static void _print_header(char param_type, char *param_value);

static void _print_body(char body_buf[], int body_len);

int server_initialize(server_t *self, const char* hostname, const char* port){
    socket_t socket;
    socket_initialize(&socket, INITIAL_FD);
    self -> socket = socket;
	if(socket_bind_listen(&self -> socket, hostname, port) == -1){
        return 1;
    }
    return 0;
}

static int _server_accept(server_t *self, socket_t *peer_socket){
    int accept_fd = socket_accept(&self -> socket);
    if (accept_fd == -1){
        return 1;
    }
    socket_initialize(peer_socket, accept_fd);
    return 0;
}

int server_execute(server_t *self){
    socket_t peer_socket;
    if (_server_accept(self, &peer_socket) == 1){
        socket_destroy(&peer_socket);
        return 1;
    }

    int status;
    do{
        status = _recv_message(&peer_socket);
        if (status == 1){
            socket_destroy(&peer_socket);
            return 1;
        }
    }while(status != 0);

    socket_destroy(&peer_socket);
    return 0;
}

int server_destroy(server_t *self){
    if(socket_destroy(&self -> socket) == 1)
        return 1;
    return 0;
}

static int _recv_message(socket_t *peer_socket){
    char initial_buf[INITIAL_HEADER_BYTES];
    int body_len, header_len, status;
    status = socket_receive(peer_socket, initial_buf, INITIAL_HEADER_BYTES);
    if (status == 1)
        return 1;
    else if (status == 0)
        return 0;

    memcpy(&body_len, &initial_buf[4], sizeof(int));
    memcpy(&header_len, &initial_buf[12], sizeof(int));
    body_len = ntohl(body_len);
    header_len = ntohl(header_len);
    char header_buf[header_len], body_buf[body_len];
    socket_receive(peer_socket, header_buf, header_len);
    socket_receive(peer_socket, body_buf, body_len);

    _print_message(initial_buf, header_buf, header_len);
    if (body_len > 0)
        _print_body(body_buf, body_len);
    printf("\n");
    char response[RESPONSE_LEN] = "OK\n";
    socket_send(peer_socket, response, RESPONSE_LEN, 0);
    return status;
}

static void _print_message(char initial_buf[], char header_buf[], int header_len){
    char param_type; 
    char *param_value = calloc(1, sizeof(char));
    param_type = header_buf[0];
    int param_len, i = 0, j = 0;

    printf("* Id: 0x%08x\n", (int) initial_buf[11]);
    while (i < header_len){
        param_type = header_buf[i];
        if (param_type == 8)
            break;
        memset(param_value, '\0', strlen(param_value));
        i += 7;
        param_len = (int) header_buf[i];
        i++;
        param_value = realloc(param_value, param_len + 1);
        while (j < param_len){
            param_value[j] = header_buf[i];
            j++;
            i++;
        }
        param_value[j] = '\0';
        j = 0;
        i++;
        while (header_buf[i] == '\0')
            i++;
        _print_header(param_type, param_value);
    }

    free(param_value);
}

static void _print_header(char param_type, char *param_value){
    switch (param_type){
        case 6:
            printf("* Destino: ");
        break;
        case 1:
            printf("* Path: ");
        break;
        case 2:
            printf("* Interfaz: ");
        break;
        case 3:
            printf("* Método: ");
    }
    printf("%s\n", param_value);
}

static void _print_body(char body_buf[], int body_len){ 
    printf("* Parámetros: \n");
    char *param_value = calloc(1, sizeof(char));
    int i = 0, j = 0, param_len;

    while (i < body_len - 1){
        i += 3;
        param_len = (int) body_buf[i];
        memset(param_value, 0, strlen(param_value));
        i++;
        param_value = realloc(param_value, param_len + 1);
        while (j < param_len){
            param_value[j] = body_buf[i];
            j++;
            i++;
        }
        param_value[j] = '\0';
        j = 0;
        i++;
        printf("    * %s\n", param_value);
    }

    free(param_value);
}










