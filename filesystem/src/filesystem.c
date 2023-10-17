
#include "filesystem.h"
#include<readline/readline.h>
int main(int argc, char* argv[]) {

	char *ruta_config = argv[1];

	config = cargar_config(ruta_config);

    logger = log_create("./filesystem.log", "FILESYSTEM", true, LOG_LEVEL_INFO);


    log_info(logger, "Soy el filesystem!");
    //obtener datos de .config
	obtener_configuracion();
	iniciar_consola();

	terminar_programa(conexion_memoria, logger, config);
    return EXIT_SUCCESS;
}


void obtener_configuracion(){
	ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
	puerto_escucha= config_get_string_value(config,"PUERTO_ESCUCHA");
}


void iniciar_consola(){
	logger_consola_filesystem = log_create("./filesystemConsola.log", "consola", 1, LOG_LEVEL_INFO);
	char* valor;

	while(1){
		log_info(logger_consola_filesystem,"ingrese la operacion que deseas realizar"
				"\n 1. generar conexion"
				"\n 2. enviar mensaje"
				"\n 3. iniciarComoServidor");
		valor = readline("<");
		switch (*valor) {
			case '1':
				log_info(logger_consola_filesystem, "generar conexion con memoria\n");
				conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);
				break;
			case '2':
				log_info(logger_consola_filesystem, "enviar mensaje a memoria\n");
				enviar_mensaje("filesystem a memoria", conexion_memoria);
				break;
			case '3':
				log_info(logger_consola_filesystem, "se inicio el servidor\n");
				iniciar_servidor(puerto_escucha);
				break;
			default:
				log_info(logger_consola_filesystem,"no corresponde a ninguno");
				exit(2);
		}
		free(valor);
	}

}


int iniciar_servidor_file_system(char *puerto){
	int servidor_fd = iniciar_servidor(puerto);
	log_info(logger, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_cliente(servidor_fd);

	t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
			log_info(logger, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;

}
void iterator(char* value) {
	log_info(logger,"%s", value);
}
