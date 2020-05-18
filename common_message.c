#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common_message.h"
#include "common_swapper.h"

//recorre el header del mensaje traducido,
//recuperando de él los parámetros y sus tipos
//cada vez que consigue uno, lo envía a la función _print_message para
//que esta lo muestre por pantalla
static void _print_header(char initial_buf[], char *header_buf);

static void _print_message(char param_type, char *param_value);

static void _print_body(char *body_buf, int body_len);

void message_print(char initial_buf[], char *header_buf, char *body_buf){
	_print_header(initial_buf, header_buf);
	int body_len;
	memcpy(&body_len, &initial_buf[4], sizeof(int));
    body_len = swapper_swap_bytes(body_len);
	if(body_len > 0)
		_print_body(body_buf, body_len);
	printf("\n");
}

static void _print_header(char initial_buf[], char *header_buf){
    char param_type, *param_value;
    int param_len, header_len, i = 0, j = 0;
    memcpy(&header_len, &initial_buf[12], sizeof(int));
    header_len = swapper_swap_bytes(header_len);

    printf("* Id: 0x%08x\n", (int) initial_buf[8]);
    while (i < header_len){
        param_type = header_buf[i];
        if (param_type == 8)
            break;
        i += 4;
        param_len = (int) header_buf[i];
        i += 4;
        param_value = malloc(param_len + 1);
        while (j < param_len){
            param_value[j] = header_buf[i];
            j++;
            i++;
        }
        param_value[j] = '\0';
        j = 0;
        i++;
        while (i < header_len && header_buf[i] == '\0')
            i++;
        _print_message(param_type, param_value);
        free(param_value);
    }
}

static void _print_message(char param_type, char *param_value){
    switch (param_type){
        case 6:
            printf("* Destino: ");
        break;
        case 1:
            printf("* Ruta: ");
        break;
        case 2:
            printf("* Interfaz: ");
        break;
        case 3:
            printf("* Metodo: ");
    }
    printf("%s\n", param_value);
}

static void _print_body(char *body_buf, int body_len){ 
    printf("* Parametros:\n");
    char *param_value;
    int i = 0, j = 0, param_len;

    while (i < body_len - 1){
        param_len = (int) body_buf[i];
        i += 4;
        param_value = malloc(param_len + 1);
        while (j < param_len){
            param_value[j] = body_buf[i];
            j++;
            i++;
        }
        param_value[j] = '\0';
        j = 0;
        i++;
        printf("    * %s\n", param_value);
        free(param_value);
    }
}
