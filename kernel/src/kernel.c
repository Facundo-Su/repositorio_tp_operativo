#include "kernel.h"
#include<readline/readline.h>

int main(int argc, char **argv){


	char *rutaConfig = "kernel.config";
	config = cargar_config(rutaConfig);

    logger = log_create("./kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    log_info(logger, "Soy el Kernel!");

    obtener_configuracion();
    iniciar_recurso();
    iniciar_consola();

    //envio de mensajes

    //error
    //paquete(conexion_memoria);

    terminar_programa(conexion_memoria, logger, config);
    terminar_programa(conexion_cpu, logger, config);
    terminar_programa(conexion_file_system, logger, config);


    return EXIT_SUCCESS;
}


void procesar_conexion(void *conexion1){
	int *conexion = (int*)conexion1;
	int cliente_fd = *conexion;

	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		log_info(logger,"hola");
		t_pcb* pcb_aux;
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
		case EJECUTAR_SLEEP:
			pcb_aux = recibir_pcb(cliente_fd);
			int tiempo = atoi(obtener_mensaje(cliente_fd));
			sleep(tiempo);
			break;
		case EJECUTAR_WAIT:
			pcb_aux = recibir_pcb(cliente_fd);
			char * nombre_recurso = obtener_mensaje(cliente_fd);
			ejecutar_wait(nombre_recurso,pcb_aux);
			break;
		case EJECUTAR_SIGNAL:
			pcb_aux = recibir_pcb(cliente_fd);
			char * nombre_recurso2 = obtener_mensaje(cliente_fd);
			ejecutar_signal(nombre_recurso2,pcb_aux);
			break;
		case EJECUTAR_F_TRUNCATE:
			log_info(logger,"me llegaron la instruccion ejecutar f truncate del cpu");
			break;
		case FINALIZAR:
			pcb_aux = recibir_pcb(cliente_fd);
			enviar_pcb(pcb_aux,conexion_memoria,FINALIZAR);
			break;

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
	logger_consola = log_create("./kernelConsola.log", "consola", 1, LOG_LEVEL_INFO);
	char* variable;

	while(1){
		log_info(logger_consola,"ingrese la operacion que deseas realizar"
				"\n 1. iniciar Proceso"
				"\n 2. finalizar proceso"
				"\n 3. iniciar Planificacion"
				"\n 4. detener Planificacion"
				"\n 5. modificar grado multiprogramacion"
				"\n 6. hacer que cpu mande mensaje a memoria"
				"\n 7. generar conexion"
				"\n 8. enviar mensaje");
		variable = readline(">");

		switch (*variable) {
			case '1':
				log_info(logger_consola, "ingrese la ruta");
				char* ruta = readline(">");
				log_info(logger_consola, "ingrese el tamanio");
				int size = atoi(readline(">"));
				log_info(logger_consola, "ingrese el prioridad");
				int prioridad = atoi(readline(">"));

				iniciar_proceso(ruta,size,prioridad,contador_pid);
				break;
			case '2':
				log_info(logger_consola, "ingrese pid");
				char* valor = readline(">");
				int valorNumero = atoi(valor);
				finalizar_proceso(valorNumero);
				break;
			case '3':
				iniciar_planificacion();
				break;
			case '4':
				detener_planificacion();
				break;
			case '5':
				modificar_grado_multiprogramacion();
				break;
			case '6':
				listar_proceso_estado();
				break;
			case '7':
				generar_conexion();
				break;
			case '8':
				enviar_mensaje_kernel();
				break;
			case '9':
				crear_pcb(FIFO);
				log_info(logger,"%i",list_size(cola_new));
				enviar_pcb(queue_pop(cola_new),conexion_cpu,RECIBIR_PCB);
				break;
			default:
				log_info(logger_consola,"no corresponde a ninguno");
				exit(2);
		}
		free(variable);


	}

}

void iniciar_recurso(){
	lista_pcb=list_create();
	cola_new = queue_create();
	cola_ready = queue_create();
	lista_recursos = list_create();
	sem_init(&grado_multiprogramacion, 0, grado_multiprogramacion_ini);
	sem_init(&mutex_cola_new, 0, 1);
	sem_init(&mutex_cola_ready,0,1);

}

void enviar_mensaje_kernel() {
	log_info(logger_consola,"ingrese q que modulos deseas mandar mensaje"
			"\n 1. modulo memoria"
			"\n 2. modulo cpu"
			"\n 3. modulo filesystem");
    char *valor = readline(">");
	switch (*valor) {
		case '1':
	        enviar_mensaje("kernel a memoria", conexion_memoria);
	        log_info(logger_consola,"mensaje enviado correctamente\n");
			break;
		case '2':
	        enviar_mensaje("kernel a cpu", conexion_cpu);
	        log_info(logger_consola,"mensaje enviado correctamente\n");
			break;
		case '3':
	        enviar_mensaje("kernel a filesystem", conexion_file_system);
	        log_info(logger_consola,"mensaje enviado correctamente\n");
			break;
		default:
			log_info(logger_consola,"no corresponde a ninguno\n");
			break;
	}
}

void generar_conexion() {
	pthread_t conexion_memoria_hilo;
	pthread_t conexion_file_system_hilo;
	pthread_t conexion_cpu_hilo;

	log_info(logger_consola,"ingrese q que modulos deseas conectar"
			"\n 1. modulo memoria"
			"\n 2. modulo filesystem"
			"\n 3. modulo cpu");

    char *valor = readline(">");
	switch (*valor) {
		case '1':

			conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);
	        log_info(logger_consola,"conexion generado correctamente\n");
			break;
		case '2':
			conexion_file_system = crear_conexion(ip_filesystem, puerto_filesystem);
	        log_info(logger_consola,"conexion generado correctamente\n");
			break;
		case '3':
			conexion_cpu = crear_conexion(ip_cpu, puerto_cpu_dispatch);
	        log_info(logger_consola,"conexion generado correctamente\n");
			pthread_create(&conexion_cpu_hilo,NULL,(void*) procesar_conexion,(void *)&conexion_cpu);
			break;
		default:
			log_info(logger_consola,"no corresponde a ninguno\n");
			break;
	}

}


//hilo que espere consola,
void iniciar_proceso(char* archivo_test,int size,int prioridad,int pid){

	//char* prueba = ruta_archivo_test;
	//string_append(*prueba, archivo_test);

	char*ruta_a_testear = archivo_test;

	crear_pcb(FIFO);

	op_code op = INICIAR_PROCESO;
	t_paquete* paquete =crear_paquete(op);
	agregar_a_paquete(paquete, ruta_a_testear, sizeof(ruta_a_testear));
	agregar_a_paquete(paquete, &size ,sizeof(int));
	agregar_a_paquete(paquete, &prioridad, sizeof(int));
	agregar_a_paquete(paquete, &pid, sizeof(int));

	enviar_paquete(paquete, conexion_memoria);
	crear_pcb(prioridad);
	//free(prueba);
	eliminar_paquete(paquete);
	free(ruta_a_testear);

}

void crear_pcb(t_planificador prioridad){
	t_pcb* pcb = malloc(sizeof(pcb));
	pcb->pid= contador_pid;
	pcb->prioridad = prioridad;
	t_contexto_ejecucion* contexto = crear_contexto();
	pcb->contexto =contexto;
	//pcb->tabla_archivo_abierto;
	pcb->estado=NEW;
	contador_pid++;

	agregar_a_cola_new(pcb);
}

t_contexto_ejecucion* crear_contexto(){
	t_contexto_ejecucion* contexto = malloc(sizeof(t_contexto_ejecucion));
	contexto->pc =NULL;
	t_registro_cpu* registro = crear_registro();
	contexto->registros_cpu = registro;
	return contexto;
}

t_registro_cpu* crear_registro(){
	t_registro_cpu* reg = malloc(sizeof(t_registro_cpu));
    memset(reg->AX, 0, sizeof(reg->AX));
    memset(reg->BX, 0, sizeof(reg->BX));
    memset(reg->CX, 0, sizeof(reg->CX));
    memset(reg->DX, 0, sizeof(reg->DX));
	return reg;
}


void agregar_a_cola_new(t_pcb* pcb){
	sem_wait(&mutex_cola_new);
	queue_push(cola_new,pcb);
	sem_post(&mutex_cola_new);
	log_info(logger,"El proceso [%d] fue agregado a la cola new",pcb->pid);
}

t_pcb* quitar_de_cola_new(){
	sem_wait(&mutex_cola_new);
	t_pcb* pcb=queue_pop(cola_new);
	sem_post(&mutex_cola_new);
	log_info(logger,"El proceso [%d] fue quitado de la cola new",pcb->pid);
	return pcb;
}
void agregar_a_cola_ready(t_pcb* pcb){
	sem_wait(&mutex_cola_ready);
	queue_push(cola_ready,pcb);
	pcb->estado=READY;
	sem_post(&mutex_cola_ready);
	log_info(logger,"El proceso [%d] fue agregado a la cola ready",pcb->pid);
}
t_pcb* quitar_de_cola_ready(){
	sem_wait(&mutex_cola_ready);
	t_pcb* pcb=queue_pop(cola_ready);
	sem_post(&mutex_cola_ready);
	log_info(logger,"El proceso [%d] fue quitado de la cola ready",pcb->pid);
	return pcb;
}
void agregar_a_cola_ejecucion(t_pcb* pcb){
	sem_wait(&mutex_cola_ejecucion);
	queue_push(cola_ready,pcb);
	pcb->estado=RUNNING;
	sem_post(&mutex_cola_ejecucion);
	log_info(logger,"El proceso [%d] fue agregado a la cola ejecucion",pcb->pid);
}
t_pcb* quitar_de_cola_ejecucion(){
	sem_wait(&mutex_cola_ejecucion);
	t_pcb* pcb=queue_pop(cola_ejecucion);
	sem_post(&mutex_cola_ejecucion);
	log_info(logger,"El proceso [%d] fue quitado de la cola ejecucion",pcb->pid);
	return pcb;
}
void planificador_largo_plazo(){
	while(1){
		while(!queue_is_empty(cola_new)){
			sem_wait(&grado_multiprogramacion);
			t_pcb* pcb =quitar_de_cola_new();
			agregar_a_cola_ready(pcb);
		}
	}
}
void planificador_corto_plazo(){
//  Desmarcar para probar los planificadores
	/*planificador = PRIORIDADES;
	t_pcb pcb1 = {
		1,
		1,
		1,
		NULL,
		NULL,
		NEW,
		NULL,
		NULL
	};
	t_pcb pcb2 = {
		2,
		4,
		1,
		NULL,
		NULL,
		NEW,
		NULL,
		NULL
	};
	t_pcb pcb3 = {
		3,
	    2,
	    1,
		NULL,
		NULL,
		NEW,
		NULL,
	    NULL
	};
    t_pcb *ppcb1;
	ppcb1 = &pcb1;
	t_pcb *ppcb2;
	ppcb2 = &pcb2;
	t_pcb *ppcb3;
	ppcb3 = &pcb3;
	agregar_a_cola_ready(ppcb1);
	agregar_a_cola_ready(ppcb2);
	agregar_a_cola_ready(ppcb3);
*/
	while(1){
			while(!queue_is_empty(cola_ready)){
				switch(planificador){
				case FIFO:
					log_info(logger,"Planificador FIFO");
					de_ready_a_fifo();
					break;
				case ROUND_ROBIN:
					log_info(logger,"Planificador Round Robin");

					de_ready_a_round_robin();
					break;
				case PRIORIDADES:
					log_info(logger,"Planificador Prioridades");
					de_ready_a_prioridades();
					break;
				}
			}
		}
}
void de_ready_a_fifo(){
	t_pcb* pcb =quitar_de_cola_ready();
	pcb->estado=RUNNING;
	log_info(logger,"El proceso [%d] fue agregado a la cola ejecucion",pcb->pid);
	enviar_pcb(pcb,conexion_cpu,RECIBIR_PCB);
}

//TODO Revisar el if que crashea
void de_ready_a_round_robin(){

	if(!queue_is_empty(cola_ready)){
		log_info(logger,"cantidad de elemento en cola es %i",queue_size(cola_ready));
		t_pcb* pcb = queue_peek(cola_ejecucion);
		if(pcb->tiempo_cpu > quantum){
		 quitar_de_cola_ejecucion();
		 de_ready_a_fifo();
		} else {
		 de_ready_a_fifo();
	    }
	} else {
		de_ready_a_fifo();
	}
}

void de_ready_a_prioridades(){
	list_sort(cola_ready->elements,comparador_prioridades);
	t_pcb* pcb = quitar_de_cola_ready();
	pcb->estado=RUNNING;
	log_info(logger,"“El PID: %d paso de ready a running ”",pcb->pid);
	enviar_pcb(pcb,conexion_cpu,RECIBIR_PCB);
}

bool comparador_prioridades(void* caso1,void* caso2){
	t_pcb* pcb1 = ((t_pcb*) caso1);
	t_pcb* pcb2 = ((t_pcb*) caso2);
	if(pcb1->prioridad > pcb2->prioridad){
		return true;
	} else return false;
}

bool controlador_multi_programacion(){
	return list_size(lista_pcb)<grado_multiprogramacion_ini;
}





t_contexto_ejecucion* obtener_contexto(char* archivo){
	t_contexto_ejecucion *estructura_retornar ;

	return estructura_retornar;
}

// ver como pasar int TODO
void finalizar_proceso(int pid){

	t_paquete * paquete = crear_paquete(FINALIZAR);
	agregar_a_paquete(paquete, &pid, sizeof(int));
	enviar_paquete(paquete, conexion_memoria);

	eliminar_paquete(paquete);
	free(paquete);




	//int posicion= buscarPosicionQueEstaElPid(pid);
	//t_pcb* auxiliar =list_remove(cola_new->elements,posicion);
	//liberarMemoriaPcb(auxiliar);
}

void liberarMemoriaPcb(t_pcb* pcbABorrar){
		free(pcbABorrar->contexto);
		free(pcbABorrar->tabla_archivo_abierto);
		free(pcbABorrar);
}

int buscarPosicionQueEstaElPid(int valor){
	int cantidad= list_size(lista_pcb);
	t_pcb* elemento ;
	for(int i=0;i<cantidad;i++){
		elemento = list_get(lista_pcb,cantidad);
		if(elemento->pid == valor){
			return cantidad;
		}
	}

	return -1;
}

void iniciar_planificacion(){
	log_info(logger_consola,"inicio el proceso de planificacion");
	planificador_corto_plazo();
}


void detener_planificacion(){

}
void modificar_grado_multiprogramacion(){

}
void listar_proceso_estado(){
	t_paquete * aux = crear_paquete(CPU_ENVIA_A_MEMORIA);
	enviar_paquete(aux, conexion_cpu);
}


void obtener_configuracion(){

    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    ip_filesystem = config_get_string_value(config, "IP_FILESYSTEM");
    ip_cpu = config_get_string_value(config, "IP_CPU");
    char *algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    asignar_algoritmo(algoritmo);
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    puerto_filesystem = config_get_string_value(config, "PUERTO_FILESYSTEM");
    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
    quantum = config_get_int_value(config, "QUANTUM");
    grado_multiprogramacion_ini = config_get_int_value(config, "GRADO_MULTIPROGRAMACION_INI");
    recursos = config_get_array_value(config, "RECURSOS");
    char **instancias = config_get_array_value(config, "INSTANCIAS_RECURSOS");
    instancias_recursos = string_to_int_array(instancias);
    string_array_destroy(instancias);
}

void asignar_algoritmo(char *algoritmo){
	if (strcmp(algoritmo, "FIFO") == 0) {
		planificador = FIFO;
	} else if (strcmp(algoritmo, "HRRN") == 0) {
		planificador = ROUND_ROBIN;
	}else if(strcmp(algoritmo, "PRIORIDADES")==0){
		planificador = PRIORIDADES;
	}else{
		log_error(logger, "El algoritmo no es valido");
	}
}

int* string_to_int_array(char** array_de_strings){
	int count = string_array_size(array_de_strings);
	int *numbers = malloc(sizeof(int) * count);
	for(int i = 0; i < count; i++){
		int num = atoi(array_de_strings[i]);
		numbers[i] = num;
	}
	return numbers;
}

void ejecutar_wait(char* recurso_a_encontrar, t_pcb * pcb){

    bool encontrar_recurso(void * recurso){
          t_recurso* un_recurso = (t_recurso*)recurso;
          return strcmp(un_recurso->nombre, recurso_a_encontrar) == 0;
    }

    t_recurso *recurso_encontrado = list_find(lista_recursos, encontrar_recurso);
        if(recurso_encontrado != NULL){
            if(recurso_encontrado->instancias >0 ){
            	pcb = agregar_recurso_pcb(pcb, recurso_a_encontrar);
            	int posicion = buscar_posicion_lista_recurso(lista_recursos, recurso_encontrado);
                recurso_encontrado->instancias -=1;
                list_replace(lista_recursos,posicion,recurso_encontrado);
            }else{
                queue_push(recurso_encontrado->cola_bloqueados,pcb);
            }
        }else{
            enviar_pcb(pcb,conexion_memoria,FINALIZAR);
        }
}

t_pcb*agregar_recurso_pcb(t_pcb*pcb, char*nombre){
	bool encontrar_recurso(void * recurso){
	          t_recurso_pcb* un_recurso = (t_recurso_pcb*)recurso;
	          return strcmp(un_recurso->nombre, nombre) == 0;
	    }
	    t_recurso_pcb *recurso_encontrado = list_find(pcb->recursos, encontrar_recurso);
	    if(recurso_encontrado != NULL){
	    	int posicion = buscar_posicion_lista_recurso_pcb(pcb->recursos,recurso_encontrado);
	    	recurso_encontrado->instancias++;
	    	list_replace(pcb->recursos,posicion,recurso_encontrado);
	    }else{
	    	t_recurso_pcb*recurso_nuevo;
	    	recurso_nuevo=crear_recurso(nombre);
	    	list_add(pcb->recursos, recurso_nuevo);
	    }
	    return pcb;
}
t_pcb*quitar_recurso_pcb(t_pcb*pcb, char*nombre){
	bool encontrar_recurso(void * recurso){
	          t_recurso_pcb* un_recurso = (t_recurso_pcb*)recurso;
	          return strcmp(un_recurso->nombre, nombre) == 0;
	    }
	    t_recurso_pcb *recurso_encontrado = list_find(pcb->recursos, encontrar_recurso);
	    	if(recurso_encontrado != NULL){
				int posicion = buscar_posicion_lista_recurso_pcb(pcb->recursos,recurso_encontrado);
				recurso_encontrado->instancias--;
				list_replace(pcb->recursos,posicion,recurso_encontrado);
	    	}
	    return pcb;
}

t_recurso_pcb*crear_recurso(char*nombre){
	t_recurso_pcb*recurso_nuevo=malloc(sizeof(t_recurso_pcb));
	recurso_nuevo->nombre=nombre;
	recurso_nuevo->instancias=1;
	return recurso_nuevo;
}
void ejecutar_signal(char* recurso_a_encontrar, t_pcb * pcb){
    bool encontrar_recurso(void * recurso){
              t_recurso* un_recurso = (t_recurso*)recurso;
              return strcmp(un_recurso->nombre, recurso_a_encontrar) == 0;
    }
    bool encontrar_recurso_pcb(void * recurso){
                  t_recurso_pcb* un_recurso = (t_recurso_pcb*)recurso;
                  return strcmp(un_recurso->nombre, recurso_a_encontrar) == 0;
        }
    t_recurso *recurso_encontrado = list_find(lista_recursos, encontrar_recurso);
    t_recurso_pcb *recurso_pcb = list_find(pcb->recursos, encontrar_recurso_pcb);
    if(recurso_encontrado != NULL){
    	if(recurso_pcb->instancias > 0){
    	 int posicion = buscar_posicion_lista_recurso(lista_recursos, recurso_encontrado);
    	 recurso_encontrado->instancias ++;
    	 list_replace(lista_recursos,posicion,recurso_encontrado);
    	 pcb = quitar_recurso_pcb(pcb,recurso_a_encontrar);
    	}else{
    	enviar_pcb(pcb,conexion_memoria,FINALIZAR);
    	}
    }else{
        enviar_pcb(pcb,conexion_memoria,FINALIZAR);
    }
}


int buscar_posicion_lista_recurso_pcb(t_list*lista, t_recurso_pcb *recurso){
	int cantidad= list_size(lista);
	t_recurso_pcb* elemento ;
	for(int i=0;i<cantidad;i++){
		elemento = list_get(lista,cantidad);
		if(elemento->nombre == recurso->nombre){
			return cantidad;
		}
	}
	return -1;
}
int buscar_posicion_lista_recurso(t_list*lista, t_recurso *recurso){
	int cantidad= list_size(lista);
	t_recurso* elemento ;
	for(int i=0;i<cantidad;i++){
		elemento = list_get(lista,cantidad);
		if(elemento->nombre == recurso->nombre){
			return cantidad;
		}
	}
	return -1;
}

