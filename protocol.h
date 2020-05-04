#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

typedef unsigned char byte;

typedef struct{
	byte *header;
	byte *signature;
	byte *body;
	uint32_t header_len;
	uint32_t signature_len;
	uint32_t body_len;
	uint32_t signature_padding;
}protocol_t;

//inicializa el protocolo con determinados valores fijos para todos los mensajes
//por ejemplo, la versión de protocolo (1), flags (0), etcétera
//ademas, asigna el ID de mensaje
void protocol_initialize(protocol_t *self, uint32_t message_id);

//hilo conductor de la traducción del mensaje al protocolo
//retorna el número de bytes de padding de la firma
int protocol_translate(protocol_t *self, byte **message_buffer);

//lee el mensaje hasta encontrar un espacio, en cuyo caso llama a la función _write_header
//vuelve una vez que encuentra el caracter '('
//retorna la posición de la primera letra del primer parámetro (o del caracter ')' si no hay ninguno)
static int _parse_header(protocol_t *message, byte *message_buffer);

//escribe el parámetro en el header
static void _write_header(protocol_t *self, byte *string, int space_count);
//
static void _header_assign(protocol_t *self, uint32_t old_header_len, int space_count);

static int _padding_space(int n);

static void _parse_body(protocol_t *self, byte *message_buffer, int i);

static void _signature_initialize(protocol_t *self);

static void _signature_update(protocol_t *self);

static void _signature_padding(protocol_t *self, int param_count);

static void _write_body(protocol_t *self, byte *string);

static int _assemble_message(protocol_t *self, byte **message_buffer);

void protocol_destroy(protocol_t *self);

#endif
