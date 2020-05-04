#include <stdio.h>
#include <stdlib.h>
#include "client.h"

int main(int argc, char* argv[]){
	FILE* input;

	if(argc == 3){
		input = stdin;
	} else if (argc == 4){
		input = fopen(argv[3], "rt");
	} else {
		printf("Modo de uso: ./client <hostname> <port> <nombre de archivo>\n");
		return 1;
	}

	const char* hostname = argv[1];
	const char* port = argv[2];

	client_t client;

	if(client_initialize(&client, hostname, port) == 1)
		return 1;
	if(client_execute(&client, input) == 1)
		return 1;

	client_destroy(&client);
	fclose(input);

	return 0;
}
