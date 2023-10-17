#include "conexion.h"




void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void mandar_a_memoria(char* archivo, int size, int socket_cliente) {
    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = ENVIARRUTAPARAINICIAR;
    paquete->buffer = malloc(sizeof(t_buffer));

    // Calcular el tamaño del string de la ruta
    int longitud_ruta = strlen(archivo) + 1;

    // Calcular el tamaño total del buffer
    paquete->buffer->size = longitud_ruta + sizeof(int) + sizeof(int);

    // Asignar memoria para el buffer
    paquete->buffer->stream = malloc(paquete->buffer->size);

    // Copiar la longitud de la ruta y el archivo
    memcpy(paquete->buffer->stream, &longitud_ruta, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), archivo, longitud_ruta);

    // Copiar el tamaño
    memcpy(paquete->buffer->stream + sizeof(int) + longitud_ruta, &size, sizeof(int));

    int bytes = paquete->buffer->size + 2*sizeof(int);

    void* a_enviar = serializar_paquete(paquete, bytes);

    send(socket_cliente, a_enviar, bytes, 0);

    //free(a_enviar);
    eliminar_paquete(paquete);

}





int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family,server_info->ai_socktype,server_info->ai_protocol);
	// Ahora que tenemos el socket, vamos a conectarlo
	setsockopt(socket_cliente, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void enviar_char_dinamico(char* mensaje, int socket_cliente, op_code operacion)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = operacion;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}



void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(op_code cod_op)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = cod_op;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
	/* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config)
	  con las funciones de las commons y del TP mencionadas en el enunciado */
}

t_log* logger;

int iniciar_servidor(char *puerto)
{

	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol);
	// Asociamos el socket a un puerto


	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);
	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");
	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{

	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}
char* obtener_mensaje(int socket_cliente)
{

	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	//log_info(logger, "Me llego el mensaje %s", buffer);
	return buffer;
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

t_config* cargar_config(char *ruta){
	t_config* config = config_create(ruta);
    if (config == NULL) {
        printf("No se encontró el archivo de configuración");
         exit(1);
    }
    return config;
}


//------------------------------------------------------------------------------------//
//envir contexto
void enviar_pcb(t_pcb* pcb, int conexion,op_code operacion){
	t_paquete* paquete = crear_paquete(operacion);
	empaquetar_pcb(paquete, pcb);
	enviar_paquete(paquete, conexion);

	eliminar_paquete(paquete);
}



//recibir_pcb
t_pcb* recibir_pcb(int socket_cliente){
	t_pcb* pcb = malloc(sizeof(t_pcb));
	t_list*paquete = recibir_paquete(socket_cliente);
	pcb = desempaquetar_pcb(paquete);
	return pcb;
}

//----------------------------------------------------------------------------------------------//
void empaquetar_pcb(t_paquete* paquete, t_pcb* pcb){

	agregar_a_paquete(paquete, &(pcb->pid), sizeof(int));
	agregar_a_paquete(paquete, &(pcb->estado), sizeof(t_estado));

	empaquetar_contexto_ejecucion(paquete, pcb->contexto);

	//empaquetar_instrucciones(paquete, pcb->lista_instruciones);

}

void empaquetar_contexto_ejecucion(t_paquete* paquete, t_contexto_ejecucion* contexto){

	agregar_a_paquete(paquete, &(contexto->pc), sizeof(int));

	empaquetar_registro(paquete,contexto->registros_cpu);
}

void empaquetar_registro(t_paquete* paquete, t_registro_cpu* registroCpu){
	agregar_a_paquete(paquete,&(registroCpu->AX), strlen(registroCpu->AX)+1);
	agregar_a_paquete(paquete,&(registroCpu->BX), strlen(registroCpu->BX)+1);
	agregar_a_paquete(paquete,&(registroCpu->CX), strlen(registroCpu->CX)+1);
	agregar_a_paquete(paquete,&(registroCpu->DX), strlen(registroCpu->DX)+1);
}


void empaquetar_instrucciones(t_paquete* paquete, t_instruccion* instruccion){

	agregar_a_paquete(paquete, &(instruccion->nombre), sizeof(op_instrucciones));
	empaquetarParametros(paquete,instruccion->parametros);
}


void empaquetarParametros(t_paquete * paquete, t_list* parametros){
	int* cantidad_parametros = list_size(parametros);
	agregar_a_paquete(paquete, &cantidad_parametros, sizeof(cantidad_parametros));
	for(int i=0; i<cantidad_parametros; i++){
		char* parametro = list_get(parametros, i);
		agregar_a_paquete(paquete, &(parametro), strlen(parametro)+1);
	}
}

//desempaquetar
//-----------------------------------------------------------------------------------------//

t_pcb* desempaquetar_pcb(t_list* paquete){
	t_pcb* pcb = malloc(sizeof(t_pcb));
	int * pid = list_get(paquete, 0);
	pcb->pid = *pid;
	free(pid);

	t_estado* estado = list_get(paquete, 1);
	pcb->estado = *estado;
	free(estado);

	int posicion_comienzo_contexto =2;
	t_contexto_ejecucion* contexto = desempaquetar_contexto(paquete, posicion_comienzo_contexto);
	pcb->contexto = contexto;

	return pcb;
}

t_contexto_ejecucion *desempaquetar_contexto(t_list *paquete,int posicion){
	t_contexto_ejecucion *contexto = malloc(sizeof(t_contexto_ejecucion));
	int* pc = list_get(paquete,posicion);
	contexto->pc = *pc;
	free(*pc);

	t_registro_cpu * registros = desempaquetar_registros(paquete,3);
	contexto->registros_cpu = registros;
	return contexto;
}

t_registro_cpu * desempaquetar_registros(t_list * paquete,int posicion){
	t_registro_cpu *registro = malloc(sizeof(t_registro_cpu));

	char* ax = list_get(paquete,posicion);
	strcpy(registro->AX, ax);
	free(ax);

	char* bx = list_get(paquete,posicion+1);
	strcpy(registro->BX, bx);
	free(bx);

	char* cx = list_get(paquete,posicion+2);
	strcpy(registro->CX, cx);
	free(cx);

	char* dx = list_get(paquete,posicion+3);
	strcpy(registro->DX, dx);
	free(dx);

	return registro;
}


t_instruccion * desempaquetar_instrucciones(t_list* paquete){
	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
	int posicion_usado =0;

	char* nombre = list_get(paquete,posicion_usado);
	posicion_usado+=1;
	op_instrucciones operacion = convertir_a_op_instrucciones(nombre);
	instruccion->nombre = operacion;
	posicion_usado+=1;
	t_list* parametros = desempaquetar_parametros(paquete,posicion_usado);

	return instruccion;
}


t_list* desempaquetar_parametros(t_list* paquete,int posicion){
	t_list*parametros = list_create();
	int cantidad_parametro = list_get(paquete,posicion);
	posicion++;
	for(int i=0;i<cantidad_parametro;i++){
		char* parametro = list_get(paquete,posicion);
		posicion++;
		list_add(parametros,parametro);
	}
	return parametros;

}


//convertir en op_instrucciones
op_instrucciones convertir_a_op_instrucciones(char* operacion) {
    if (strcmp(operacion, "SET") == 0) {
        return SET;
    } else if (strcmp(operacion, "SUB") == 0) {
        return SUB;
    } else if (strcmp(operacion, "SUM") == 0) {
        return SUM;
    } else if (strcmp(operacion, "EXIT") == 0) {
        return EXIT;
    } else {
    	log_info(logger_consola,"no existe el valor");
    	return -1;
    }
}




