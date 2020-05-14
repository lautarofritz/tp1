#include <stdio.h>
#include "server.h"

int main(int argc, char* argv[]){
	if(argc != 2){
		printf("Modo de uso: ./server <port> \n");
		return 1;
	}

	const char* port = argv[1];

	server_t server;

	if(server_initialize(&server, NULL, port) == 1)
		return 1;

	if(server_execute(&server) == 1)
		return 1;

	server_destroy(&server);
	return 0;
}
