#ifndef SOCKET_H
#define SOCKET_H

typedef struct{
	int fd;
}socket_t;

//creado con el único propósito de cumplir con la restricción
//de 80 caracteres por línea
typedef struct addrinfo list;

//asigna el file descriptor pasado por parámetro al socket
void socket_initialize(socket_t *self, int fd);

//trata de conectar al socket al servidor
//si lo logra, asigna el file descriptor al del socket,
//reemplazando al anterior, y retorna 0
//si no concreta la conexión, retorna 1
int socket_connect(socket_t *self, const char* hostname, const char* port);

//trata de atar al socket para que pueda aceptar conexiones
//si lo logra, asigna el file descriptor al del socket,
//reemplazando al anterior
//si no lo logra, retorna 1
//si puede atarlo, lo deja escuchando conexiones entrantes
//si no puede dejarlo en este estado, retorna 1
//si ambas acciones anteriores tienen éxito, retorna 0
int socket_bind_listen(socket_t *self, const char* hostname, const char* port);

//retorna el file descriptor del peer socket usado
//para la comunicación
int socket_accept(socket_t *self);

//envía la cantidad de bytes del mensaje pasado por parámetro
//indicada por el parámetro "len"
//usa el parámetro "offset" para saber en donde ubicarse en el mensaje
//retorna 0 si el envío fue exitoso o 1 si hubo algún error durante el mismo
int socket_send(socket_t *self, char *msg, int len, int offset);

//recibe la cantidad de bytes del mensaje pasado por parámetro
//indicada por el parámetro "len"
//retorna 2 si la recepción fue exitosa, caso contrario retorna 1
//retorna 0 si la otra parte se desconectó
int socket_receive(socket_t *self, char buf[], int len);

int socket_destroy(socket_t *self);

#endif
