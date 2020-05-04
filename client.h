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

//inicializa al cliente con un socket al que primero inicializa con un 
//valor "simbólico" de -1
//luego, trata de conectarlo al servidor
//devuelve 0 si la conexión fue exitosa o 1 si falló
int client_initialize(client_t *self, const char* hostname, const char* port);

//toma las líneas del archivo y las manda al protocolo, 
//para que este las traduzca
//luego le pasa el mensaje ya traducido al socket para que este lo envíe
//retorna 0 si el envío por parte del socket fue exitoso, o 1 si hubo
//algún problema durante este
int client_execute(client_t *self, FILE* input);

int client_destroy(client_t *self);

#endif
