#ifndef KERNEL_H_
#define KERNEL_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <utils/conexion.h>

t_log* logger;
t_config* config;
t_log * logger_consola_memoria;

typedef enum{
	FIFO,
	LRU
}t_algoritmo;

typedef struct{
	char* ruta;
	int size;
}estructura_inicial;

typedef struct{
	int pid;
	t_list* instrucciones;
}t_instrucciones;

estructura_inicial* estrctura_inicial;

int conexion_filesystem;

t_list *lista_instrucciones;


char *puerto_escucha;
char *ip_file_system;
char *puerto_filesystem;
int tam_memoria;
int tam_pagina;
char *path_instrucciones;
int retraso_respuesta;
t_algoritmo algoritmo;


void iterator(char* value);
void obtener_configuraciones();
void iniciar_servidor_memoria(char*);
void iniciar_consola();
void recibir_estructura_inicial(int);
void procesar_conexion(void *);
void iniciar_recursos();
void cargar_lista_instruccion(char*,int ,int,int);
t_list* leer_pseudocodigo(FILE*);
char** parsear_instruccion(char*);
op_instrucciones asignar_cod_instruccion(char* );
bool encontrar_instrucciones(void * );


#endif /* KERNEL_H_ */

