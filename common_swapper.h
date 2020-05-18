#ifndef SWAPPER_H
#define SWAPPER_H

#include <byteswap.h>

//chequea si la máquina es big o little endian
//si es big invierte los bytes del número recibido
//si es little no hace nada
int swapper_swap_bytes(int n);

#endif