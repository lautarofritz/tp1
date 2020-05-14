#include <string.h>
#include <stdlib.h>
#include "server.h"
#include "common_socket.h"
#include "common_message.h"

//trata de asociar el socket recibido por parámetro a una conexión entrante
//si el file descriptor recibido es válido, retorna 0
//caso contrario, retorna 1
static int _server_accept(server_t *self, socket_t *peer_socket);

//recibe el mensaje del cliente en tres partes(inicio, header y body)
//si alguna de las recepciones falla, devuelve 1
//si todo sale bien, devuelve el valor almacenado en "status" (bytes leídos)
//si la conexión terminó, retorna 0
static int _recv_message(socket_t *peer_socket);

static void _send_response(socket_t *peer_socket);

int server_initialize(server_t *self, const char* hostname, const char* port){
    socket_t socket;
    socket_initialize(&socket);
    self -> socket = socket;

	if(socket_bind_listen(&self -> socket, hostname, port) == -1){
        return 1;
    }
    return 0;
}

static int _server_accept(server_t *self, socket_t *peer_socket){
    if (socket_accept(&self -> socket, peer_socket) == 1)
        return 1;
    
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
    if (socket_destroy(&self -> socket) == 1)
        return 1;
    return 0;
}

static int _recv_message(socket_t *peer_socket){
    char initial_buf[INITIAL_HEADER_BYTES];
    char *header_buf = NULL, *body_buf = NULL;
    int body_len, header_len, status;
    status = socket_receive(peer_socket, initial_buf, INITIAL_HEADER_BYTES);
    if (status == -1)
        return 1;
    else if (status == 0)
        return 0;

    memcpy(&body_len, &initial_buf[4], sizeof(int));
    memcpy(&header_len, &initial_buf[12], sizeof(int));

    header_buf = malloc(header_len);
    if (socket_receive(peer_socket, header_buf, header_len) == 1)
        return 1;

    if (body_len > 0){
        body_buf = malloc(body_len);
        if (socket_receive(peer_socket, body_buf, body_len) == 1)
            return 1;
    }

    message_print(initial_buf, header_buf, body_buf);
    _send_response(peer_socket);
    free(header_buf);
    if (body_len > 0)
        free(body_buf);
    return status;
}

static void _send_response(socket_t *peer_socket){
    char response[RESPONSE_LEN] = "OK\n";
    socket_send(peer_socket, response, RESPONSE_LEN);
}
