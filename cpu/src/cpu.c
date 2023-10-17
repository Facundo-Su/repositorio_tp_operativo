
#include "cpu.h"

int main(int argc, char* argv[]) {

	char *rutaConfig = "./cpu.config";

	config = cargar_config(rutaConfig);

    logger = log_create("./cpu.log", "CPU", true, LOG_LEVEL_INFO);
    log_info(logger, "Soy la cpu!");

    //iniciar configuraciones
	 obtener_configuracion();
	 iniciar_recurso();
	iniciar_consola();


	terminar_programa(conexion_memoria, logger, config);
    return 0;
}

void iniciar_recurso(){
	recibi_archivo=false;
}


void procesar_conexion(void *conexion1){
	int *conexion = (int*)conexion1;
	int cliente_fd = *conexion;
	t_list* lista = list_create();

	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		log_info(logger_consola_cpu,"hola");
		t_pcb* pcb_aux;
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
		case INSTRUCCIONES_A_MEMORIA:
			lista = recibir_paquete(cliente_fd);
			instruccion_a_realizar = desempaquetar_instrucciones(lista);
			hayInterrupcion = true;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return;
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return;
}


void iniciar_consola(){
	logger_consola_cpu = log_create("./cpuConsola.log", "consola", 1, LOG_LEVEL_INFO);
	char* valor;

	while(1){
		log_info(logger_consola_cpu,"ingrese la operacion que deseas realizar"
				"\n 1. generar conexion"
				"\n 2. enviar mensaje"
				"\n 3. iniciar Como Servidor"
				"\n 4. leer archivo");
		valor = readline(">");
		switch (*valor) {
			case '1':

				generar_conexion_memoria();
				break;
			case '2':
				log_info(logger_consola_cpu, "enviar mensaje a memoria\n");
				enviar_mensaje("cpu a memoria", conexion_memoria);
				break;
			case '3':
				log_info(logger_consola_cpu, "se inicio el servidor\n");
				iniciar_servidor_cpu(puerto_escucha);
				break;
			default:
				log_info(logger_consola_cpu,"no corresponde a ninguno");
				exit(2);
		}
		free(valor);
	}

}

void obtener_configuracion(){
	ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
	puerto_escucha = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
}
void iniciar_servidor_cpu(char *puerto){

	int cpu_fd = iniciar_servidor(puerto);
	log_info(logger, "Servidor listo para recibir al cliente");

	generar_conexion_memoria();

	log_info(logger, "genere conexion con memoria");

	while(1){
	    int cliente_fd = esperar_cliente(cpu_fd);
		pthread_t atendiendo_cpu;
		pthread_create(&atendiendo_cpu,NULL,(void*)atendiendo_pedido,(void *) cliente_fd);
		pthread_detach(atendiendo_cpu);
	}

}

void generar_conexion_memoria(){
	log_info(logger_consola_cpu, "generar conexion con memoria\n");
	pthread_t conexion_memoria_hilo_cpu;
	conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);
	pthread_create(&conexion_memoria_hilo_cpu,NULL,(void*) procesar_conexion,(void *)&conexion_memoria);
}

void atendiendo_pedido(int cliente_fd){
	t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			enviar_mensaje("hola", cliente_fd);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
			log_info(logger, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case ENVIARREGISTROCPU:
			t_list* valores_cpu = list_create();
			valores_cpu= recibir_paquete(cliente_fd);
			log_info(logger, "ME LLEGARON");
			break;

		case RECIBIR_PCB:
			t_pcb* pcb = recibir_pcb(cliente_fd);
			log_info(logger, "recibi el pid %i",pcb->pid);
			ejecutar_ciclo_de_instruccion(pcb);
			break;
		case CPU_ENVIA_A_MEMORIA:
			enviar_mensaje("hola capo", conexion_memoria);
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
            close(cliente_fd);
            break;
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return;
}

void ejecutar_ciclo_de_instruccion(t_pcb* pcb){
//pide a memoria
	while(!hayInterrupcion){
		fetch(pcb);
	}

}

void fetch(t_pcb* pcb){
	int pc = pcb->contexto->pc;
	int pid =pcb->pid;
	solicitar_instruccion_ejecutar_segun_pc(pc, pid);
	pcb->contexto->pc+=1;
	decode(pcb,instruccion_a_realizar);
}

void solicitar_instruccion_ejecutar_segun_pc(int pc,int pid){
	t_paquete* paquete = crear_paquete(INSTRUCCIONES_A_MEMORIA);
	agregar_a_paquete(paquete, &pc, sizeof(int));
	agregar_a_paquete(paquete, &pid, sizeof(int));
	enviar_paquete(paquete, conexion_memoria);

	while (!recibi_archivo) {
		int i=0;
	}

}

void decode(t_pcb* pcb,t_instruccion* instrucciones){
	t_estrucutra_cpu registro_aux;
	t_estrucutra_cpu registro_aux2;
	char * recurso;
	char* parametro;
	char* parametro2;
	tiempo_inicial = time(NULL);
	switch(instrucciones->nombre){
	case SET:
		parametro2= list_get(instrucciones->parametros,1);
		parametro= list_get(instrucciones->parametros,0);
		registro_aux = devolver_registro(parametro);
		setear(pcb,registro_aux,parametro2);
		log_info(logger_consola_cpu,"se termino de ejecutar la operacion del pid %i :",pcb->pid);
		//ADormir(x segundo);
		break;
	case SUB:
		parametro= list_get(instrucciones->parametros,0);
		parametro2= list_get(instrucciones->parametros,1);
		registro_aux = devolver_registro(parametro);
		registro_aux2 = devolver_registro(parametro2);
		restar(pcb, registro_aux, registro_aux2);
		log_info(logger_consola_cpu,"se termino de ejecutar la operacion del pid %i :",pcb->pid);
		break;
	case SUM:
		parametro= list_get(instrucciones->parametros,0);
		parametro2= list_get(instrucciones->parametros,1);
		registro_aux = devolver_registro(parametro);
		registro_aux2 = devolver_registro(parametro2);
		sumar(pcb, registro_aux, registro_aux2);
		log_info(logger_consola_cpu,"se termino de ejecutar la operacion del pid %i :",pcb->pid);
		break;
	case JNZ:
		parametro = list_get(instrucciones->parametros,0);
		parametro2 =list_get(instrucciones->parametros,1);
		registro_aux = devolver_registro(parametro);
		char* valorObtenido = obtener_valor(pcb, registro_aux);
		if(strcmp(valorObtenido,"0") ==0){
			int valorEntero = atoi(parametro2);
			pcb->contexto->pc =valorEntero;
		}
		break;
	case SLEEP:
		char * tiempo = list_get(instrucciones->parametros,0);
		enviar_pcb(pcb,cliente_fd,EJECUTAR_SLEEP);
		enviar_mensaje(tiempo,cliente_fd);
		break;
   case WAIT:
		recurso= list_get(instrucciones->parametros,0);
		enviar_pcb(pcb,cliente_fd,EJECUTAR_WAIT);
		enviar_mensaje(recurso,cliente_fd);
		break;
	case SIGNAL:
		recurso = list_get(instrucciones->parametros,0);
		enviar_pcb(pcb,cliente_fd,EJECUTAR_SIGNAL);
		enviar_mensaje(recurso,cliente_fd);
		break;
	case MOV_IN:
		log_info(logger_consola,"entendi el mensaje MOV_IN");
		break;
	case MOV_OUT:
		log_info(logger_consola,"entendi el mensaje MOV_OUT");
		break;
	case F_OPEN:
		log_info(logger_consola,"entendi el mensaje F_OPEN");
		break;
	case F_CLOSE:
		log_info(logger_consola,"entendi el mensaje F_CLOSE");
		break;
	case F_SEEK:
		log_info(logger_consola,"entendi el mensaje F_SEEK");
		break;
	case F_READ:
		log_info(logger_consola,"entendi el mensaje F_READ");
		break;
	case F_WRITE:
		log_info(logger_consola,"entendi el mensaje F_WRITE");
		break;
	case F_TRUNCATE:
		hayInterrupcion = true;
		log_info(logger_consola,"entendi el mensaje F_TRUNCATE");
		break;
	case EXIT:
		hayInterrupcion = true;
		enviar_pcb(pcb,cliente_fd,FINALIZAR);
		log_info(logger_consola,"entendi el mensaje EXIT");
		break;
	}
	tiempo_final = time(NULL);
	tiempo_transcurrido = difftime(tiempo_final, tiempo_inicial);
	pcb->tiempo_cpu = tiempo_transcurrido;

	recibi_archivo = false;
}

void setear(t_pcb* pcb, t_estrucutra_cpu pos, char* valor) {
    switch(pos) {
        case AX: memcpy(&(pcb->contexto->registros_cpu->AX), valor, strlen(valor)+1);
                 break;
        case BX: memcpy(&(pcb->contexto->registros_cpu->BX), valor, strlen(valor)+1);
                 break;
        case CX: memcpy(&(pcb->contexto->registros_cpu->CX), valor, strlen(valor)+1);
                 break;
        case DX: memcpy(&(pcb->contexto->registros_cpu->DX), valor, strlen(valor)+1);
                 break;
        default: log_info(logger, "Registro de destino no válido");
    }
}


//transformar en enum
t_estrucutra_cpu devolver_registro(char* registro){
	t_estrucutra_cpu v;
    if(strcmp(registro,"AX")==0){
        v = AX;
    } else if(strcmp(registro,"BX")==0){
        v = BX;
    } else if(strcmp(registro,"CX")==0){
        v = CX;
    } else if(strcmp(registro,"DX")==0){
        v = DX;
    } else {
        log_error(logger,"CUIDADO,CODIGO INVALIDO");
    }
    return v;
}

void sumar(t_pcb* pcb, t_estrucutra_cpu destino, t_estrucutra_cpu inicio) {
	char valor_destino;
	char valor_origen;

	valor_destino =obtener_valor(pcb, destino);
	valor_origen = obtener_valor(pcb, inicio);

    char resultado = valor_destino+ valor_origen;
    setear(pcb, destino, resultado);

}




void restar(t_pcb* pcb, t_estrucutra_cpu destino, t_estrucutra_cpu inicio) {
	char valor_destino;
	char valor_origen;

	valor_destino =obtener_valor(pcb, destino);
	valor_origen = obtener_valor(pcb, inicio);

    char resultado = valor_destino- valor_origen;
    setear(pcb, destino, resultado);
}



char* obtener_valor(t_pcb* pcb, t_estrucutra_cpu pos) {
    switch(pos) {
        case AX: return (char) pcb->contexto->registros_cpu->AX;
        case BX: return (char) pcb->contexto->registros_cpu->BX;
        case CX: return (char) pcb->contexto->registros_cpu->CX;
        case DX: return (char) pcb->contexto->registros_cpu->DX;
        default: log_info(logger, "Registro no reconocido"); return NULL;
    }
}


void iterator(char* value) {
	log_info(logger,"%s", value);
}
