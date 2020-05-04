#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define INITIAL_HEADER_BYTES 16
#define INITIAL_SIGNATURE_BYTES 7

//typedef unsigned char byte;

typedef struct{
	char *header;
	char *sign;
	char *body;
	uint32_t header_len;
	uint32_t sign_len;
	uint32_t body_len;
	uint32_t sign_padding;
}protocol_t;

//inicializa el protocolo con determinados valores fijos para todos los mensajes
//por ejemplo, la versión de protocolo (1), flags (0), etcétera
//ademas, asigna el ID de mensaje
void protocol_initialize(protocol_t *self, uint32_t message_id);

//hilo conductor de la traducción del mensaje al protocolo
//retorna el número de bytes de padding de la firma
int protocol_translate(protocol_t *self, char **message_buffer);

//lee el mensaje hasta encontrar un espacio, en cuyo caso llama
//a la función _write_header
//vuelve una vez que encuentra el caracter '('
//retorna la posición de la primera letra del primer parámetro
//(o del caracter ')' si no hay ninguno)

void protocol_destroy(protocol_t *self);

#endif
