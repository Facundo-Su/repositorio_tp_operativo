#include "memoria.h"


int main(int argc, char* argv[]) {


	char *rutaConfig = "./memoria.config";

	config = cargar_config(rutaConfig);

    obtener_configuraciones();

    iniciar_recursos();

    logger = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG);

    log_info(logger, "Soy el Memoria!");

    iniciar_consola();

    // Limpieza y terminación
    terminar_programa(conexion_filesystem, logger, config);

    return EXIT_SUCCESS;
}


void iniciar_recursos(){
	lista_instrucciones = list_create();
}

void iniciar_consola(){
	logger_consola_memoria = log_create("./memoriaConsola.log", "consola", 1, LOG_LEVEL_INFO);
	char* valor;

	while(1){
		log_info(logger_consola_memoria,"ingrese la operacion que deseas realizar"
				"\n 1. generar conexion"
				"\n 2. enviar mensaje"
				"\n 3. iniciar como servidor"
				"\n 4. obtener todas las instrucciones del archivos");
		valor = readline("<");
		switch (*valor) {
			case '1':
				log_info(logger_consola_memoria, "generar conexion con filesystem\n");
				conexion_filesystem = crear_conexion(ip_file_system, puerto_filesystem);
				break;
			case '2':
				log_info(logger_consola_memoria, "enviar mensaje a memoria\n");
				enviar_mensaje("memoria a fylesystem", conexion_filesystem);
				break;
			case '3':
				log_info(logger_consola_memoria, "se inicio el servidor\n");
				pthread_t atendiendo;
				pthread_create(&atendiendo,NULL,(void*)iniciar_servidor_memoria,(void *) puerto_escucha);
				iniciar_servidor_memoria(puerto_escucha);
				break;
			case '4':

				char * ruta = "./prueba.txt";
				FILE * archivo = fopen(ruta,"r");

				t_list* lista_aux_prueba =leer_pseudocodigo(archivo);
				log_info(logger_consola_memoria, "hola\n");
				log_info(logger_consola_memoria,"el resultado final de la lista es %i",list_size(lista_aux_prueba));

				t_instruccion * inst_prueba = list_get(lista_aux_prueba,0);
				char* valor_prueba = list_get(inst_prueba->parametros,0);
				log_info(logger_consola_memoria,"el resultado del cod_op es %d",inst_prueba->nombre);
				//log_info(logger_consola_memoria,"el pid es ");
				break;
			default:
				log_info(logger_consola_memoria,"no corresponde a ninguno");
				exit(2);
		}
		free(valor);
	}

}

void iterator(char* value) {
    log_info(logger, "%s", value);
}

void obtener_configuraciones() {
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    puerto_filesystem = config_get_string_value(config, "PUERTO_FILESYSTEM");
    ip_file_system = config_get_string_value(config, "IP_FILESYSTEM");
    path_instrucciones = strcat(config_get_string_value(config,"PATH_INSTRUCCIONES"),"/") ;
}

void iniciar_servidor_memoria(char *puerto) {

    int memoria_fd = iniciar_servidor(puerto);
    log_info(logger, "Servidor listo para recibir al cliente");

    while (1) {
        int cliente_fd = esperar_cliente(memoria_fd);
		pthread_t atendiendo;
		pthread_create(&atendiendo,NULL,(void*)atendiendo_pedido,(void *) cliente_fd);
		pthread_detach(atendiendo);

    }
}

void atendiendo_pedido(int cliente_fd){
	while (1) {
	            int cod_op = recibir_operacion(cliente_fd);
	            t_list * lista;
	            switch (cod_op) {
	            case MENSAJE:
	                recibir_mensaje(cliente_fd);
	                log_info(logger,"hola como estas");
	                enviar_mensaje("hola", cliente_fd);
	                break;
	            case PAQUETE:
	                lista = recibir_paquete(cliente_fd);
	                log_info(logger, "Me llegaron los siguientes valores:\n");
	                list_iterate(lista, (void*) iterator);
	                break;
	            case INICIAR_PROCESO:

	            	t_list* valorRecibido;
					valorRecibido=recibir_paquete(cliente_fd);
	   //         	recibir_estructura_Inicial(cliente_fd);
					//char* ruta = strcat(path_instrucciones,list_get(valorRecibido,0));
					int* size = list_get(valorRecibido,1);
					int* prioridad = list_get(valorRecibido,2);
					int* pid = list_get(valorRecibido,3);
					char *ruta = "./prueba.txt";

	                log_info(logger, "Me llegaron los siguientes valores de ruta: %s",ruta);
	                log_info(logger, "Me llegaron los siguientes valores de size: %i",*size);
	                log_info(logger, "Me llegaron los siguientes valores de prioridad: %i",*prioridad);
	                log_info(logger, "Me llegaron los siguientes valores de pid: %i",*pid);


	                cargar_lista_instruccion(ruta,size,prioridad,pid);
	                break;
	            case FINALIZAR:

	            	t_list* valor_pid;
	            	valor_pid= recibir_paquete(cliente_fd);
	            	int* valor = list_get(valor_pid,0);
	            	log_info(logger,"ME LLEGO EL PID CON EL VALOR %i :",*valor);
	            	//realizar_proceso_finalizar(valor);
	            	break;
	    		case INSTRUCCIONES_A_MEMORIA:

	    			t_list* lista;
	    			lista = recibir_paquete(cliente_fd);
	    			int* pc_recibido = list_get(lista,0);
	    			int* pid_recibido = list_get(lista,1);
	    			log_info(logger_consola_memoria,"me llegaron el siguiente pc %i",*pc_recibido);
	    			log_info(logger_consola_memoria,"me llegaron el siguiente pid %i",pid_recibido);

	    		    bool encontrar_instrucciones(void * instruccion){
	    		          t_instrucciones* un_instruccion = (t_instrucciones*)un_instruccion;
	    		          return strcmp(un_instruccion->pid, *pid) == 0;
	    		    }
	    			t_instruccion* instrucciones =list_find(lista_instrucciones,encontrar_instrucciones);
	    			t_paquete* paquete = crear_paquete(INSTRUCCIONES_A_MEMORIA);
	    			empaquetar_instrucciones(paquete,instrucciones);
	    			enviar_paquete(paquete, cliente_fd);
	    			break;
	            case -1:
	                log_error(logger, "El cliente se desconectó. Terminando servidor");
	                close(cliente_fd);
	                return; // Salir del bucle interno para esperar un nuevo cliente
	            default:
	                log_warning(logger, "Operación desconocida. No quieras meter la pata");
	                break;
	            }
	        }
}


void cargar_lista_instruccion(char *ruta,int size,int prioridad,int pid){
	t_instrucciones * instruccion = malloc(sizeof(t_instruccion));
	instruccion->pid = pid;
	instruccion->instrucciones = list_create();
	FILE * archivo = fopen(ruta,"r");
	t_list* auxiliar = leer_pseudocodigo(archivo);

	list_add_all(instruccion->instrucciones,auxiliar);
	log_info(logger_consola_memoria,"pepe");
	int cantidad = list_size(instruccion->instrucciones);
	log_info(logger_consola_memoria,"la lista total total es %i",cantidad);
	fclose(archivo);

}


t_list* leer_pseudocodigo(FILE* pseudocodigo){
    // Creo las variables para parsear el archivo
    char* instruccion = NULL;
    size_t len = 0;
    int cantidad_parametros;
    t_list* instrucciones_correspondiente_al_archivo = list_create();
	t_list * instrucciones_del_pcb = list_create();

    // Recorro el archivo de pseudocodigo y parseo las instrucciones
    while (getline(&instruccion, &len, pseudocodigo) != -1){


        t_instruccion *instruct = malloc(sizeof(t_instruccion));
        instruct->parametros= list_create();

    	log_info(logger_consola_memoria,"el valor es %s" ,instruccion);
        // Parseo la instruccion
        char** instruccion_parseada = parsear_instruccion(instruccion);

        if (strcmp(instruccion_parseada[0], "SET") == 0) {
            instruct->nombre = SET;
            cantidad_parametros = 2;
        }
        if (strcmp(instruccion_parseada[0], "SUB") == 0) {
            instruct->nombre = SUB;
            cantidad_parametros = 2;
        }
        if (strcmp(instruccion_parseada[0], "SUM") == 0) {
            instruct->nombre = SUM;
            cantidad_parametros = 2;
        }

        if (strcmp(instruccion_parseada[0], "JNZ") == 0) {
            instruct->nombre = JNZ;
            cantidad_parametros = 2;
        }
        if (strcmp(instruccion_parseada[0], "SLEEP") == 0) {
            instruct->nombre = SLEEP;
            cantidad_parametros = 1;
        }
        if (strcmp(instruccion_parseada[0], "WAIT") == 0) {
            instruct->nombre = WAIT;
            cantidad_parametros = 1;
        }
        if (strcmp(instruccion_parseada[0], "SIGNAL") == 0) {
            instruct->nombre = SIGNAL;
            cantidad_parametros = 1;
        }
        if (strcmp(instruccion_parseada[0], "MOV_IN") == 0) {
            instruct->nombre = MOV_IN;
            cantidad_parametros = 2;
        }
        if (strcmp(instruccion_parseada[0], "MOV_OUT") == 0) {
            instruct->nombre = MOV_OUT;
            cantidad_parametros = 2;
        }
        if (strcmp(instruccion_parseada[0], "F_OPEN") == 0) {
            instruct->nombre = F_OPEN;
            cantidad_parametros = 2;
        }
        if (strcmp(instruccion_parseada[0], "F_CLOSE") == 0) {
            instruct->nombre = F_CLOSE;
            cantidad_parametros = 1;
        }
        if (strcmp(instruccion_parseada[0], "F_SEEK") == 0) {
            instruct->nombre = F_SEEK;
            cantidad_parametros = 2;
        }
        if (strcmp(instruccion_parseada[0], "F_READ") == 0) {
            instruct->nombre = F_READ;
            cantidad_parametros = 2;
        }
        if (strcmp(instruccion_parseada[0], "F_WRITE") == 0) {
            instruct->nombre = F_WRITE;
            cantidad_parametros = 2;
        }
        if (strcmp(instruccion_parseada[0], "F_TRUNCATE") == 0) {
            instruct->nombre = F_TRUNCATE;
            cantidad_parametros = 2;
        }
        if (strcmp(instruccion_parseada[0], "EXIT") == 0) {
            instruct->nombre = EXIT;
            cantidad_parametros = 0;
        }

    	t_list* parametros = list_create();

        for(int i=1;i<cantidad_parametros+1;i++){
            list_add(parametros,instruccion_parseada[i]);
        }
        log_info(logger_consola_memoria, "codigo de operacion a ejecutar es %d",instruct->nombre);
    	char* parametro1 = list_get(parametros,0);
    	char* parametro2 = list_get(parametros,1);
    	log_info(logger_consola_memoria,"el valor es %s" ,parametro1);
    	log_info(logger_consola_memoria,"el valor es %s" ,parametro2);


        list_add_all(instruct->parametros,parametros);
    	log_info(logger_consola_memoria,"el tamanio de la lista es %i",list_size(instruct->parametros));

    	char* parametro3 = list_get(instruct->parametros,0);
    	log_info(logger_consola_memoria,"el valor de la PARAMETRO 1 es : %s",parametro3);
    	char* parametro4 = list_get(instruct->parametros,1);
    	log_info(logger_consola_memoria,"el valor de la PARAMETRO 2 es : %s",parametro4);

        list_add(instrucciones_del_pcb,instruct);
		log_info(logger_consola_memoria, "hola\n");
    }
    return instrucciones_del_pcb;

}

char** parsear_instruccion(char* instruccion){

    // Parseo la instruccion
    char** instruccion_parseada = string_split(instruccion, " ");

    // Retorno la instruccion parseada
    return instruccion_parseada;
}



void realizar_proceso_finalizar(int valor){

}

op_instrucciones asignar_cod_instruccion(char* instruccion){
	if (strcmp(instruccion, "SET") == 0) {
	        return SET;
	    }
	    if (strcmp(instruccion, "SUB") == 0) {
	        return SUB;
	    }
	    if (strcmp(instruccion, "SUM") == 0) {
	        return SUM;
	    }
	    if (strcmp(instruccion, "JNZ") == 0) {
	        return JNZ;
	    }
	    if (strcmp(instruccion, "SLEEP") == 0) {
	        return SLEEP;
	    }
	    if (strcmp(instruccion, "WAIT") == 0) {
	        return WAIT;
	    }
	    if (strcmp(instruccion, "SIGNAL") == 0) {
	        return SIGNAL;
	    }
	    if (strcmp(instruccion, "MOV_IN") == 0) {
	        return MOV_IN;
	    }
	    if (strcmp(instruccion, "MOV_OUT") == 0) {
	        return MOV_OUT;
	    }
	    if (strcmp(instruccion, "F_OPEN") == 0) {
	        return F_OPEN;
	    }
	    if (strcmp(instruccion, "F_CLOSE") == 0) {
	        return F_CLOSE;
	    }
	    if (strcmp(instruccion, "F_SEEK") == 0) {
	        return F_SEEK;
	    }
	    if (strcmp(instruccion, "F_READ") == 0) {
	        return F_READ;
	    }
	    if (strcmp(instruccion, "F_WRITE") == 0) {
	        return F_WRITE;
	    }
	    if (strcmp(instruccion, "F_TRUNCATE") == 0) {
	        return F_TRUNCATE;
	    }
	    if (strcmp(instruccion, "EXIT") == 0) {
	        return EXIT;
	    }

}

