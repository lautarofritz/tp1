#ifndef SERVER_H
#define SERVER_H

#include "common_socket.h"

#define INITIAL_HEADER_BYTES 16
#define RESPONSE_LEN 3

typedef struct {
	socket_t socket;
} server_t;

//inicializa al servidor con un socket al que primero instancia con un
//valor "simbólico" de -1
//luego, trata de atarlo y dejarlo escuchando
//devuelve 0 si estos dos pasos fueron exitosos o 1 si alguno falló
int server_initialize(server_t *self, const char* hostname, const char* port);

//primero intenta aceptar la conexión entrante
//si no lo logra, retorna 1
//luego recibe los mensajes del cliente hasta que haya un error
//o hasta que este se desconecte(idealmente)
//en el primer caso retorna 1, en el segundo devuelve 0
int server_execute(server_t *self);

int server_destroy(server_t *self);

#endif
