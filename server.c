#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <unistd.h>
#include <string.h> 


#define BUFFLEN 200
#define MaxCanalPorCliente 10
#define MaxCanales 100
#define MaxClientes 100


typedef struct
{
	int sock;
	struct sockaddr address;
	int addr_len;
} connection_t;



typedef struct
{
	char nombre[100];
	char key[25];
	char grupoClientes[MaxCanalPorCliente];
} canal;

typedef struct 
{

	int fdCliente;
	int numCliente;  // #thread
	char *nikname;
	char *name;
	char *mensaje;
	int mEstado;
	canal *CanalesClientes[MaxCanales];
	struct sockaddr address;
	int addr_len;

}cliente;

cliente *clientes[MaxClientes];
canal *chanels[MaxCanales];
cliente *buffer[BUFFLEN];
//cliente *ColaClientes[50];

int contador=0;
int entrada=0;
int salida=0;
int cont=0;


int inicializarCanales(){
	int i;

	for (i = 0; i < MaxCanales; ++i)
	{
		/* code */
		chanels[i]=NULL;

	}
	return 1;
}

int inicializarClientes(){

	int i;
	for ( i = 0; i < MaxClientes; ++i)
	{
		/* code */
		clientes[i]=NULL;

	}
	return 1;
}



void * atencionClienteProductor(void * ptr){   	
	char *mensaje;
	int read_size;
	cliente *client;
	client=(cliente*)ptr;
	client->mEstado=1;  //no procesado o no enviado: 0 no respondido, 1 respondido o no enviado
	write(client->fdCliente,"--Conectado--\n",14);
	while(1){
		
		if(client->mEstado>0){
			free(mensaje);
			mensaje=(char*)malloc(sizeof(char)*300);
			bzero(mensaje,300);	
			write(client->fdCliente,"Ingresa la peticion:",20);
			read_size=read(client->fdCliente,mensaje,259);
			if(read_size>0){
				//produce
				nivelar(mensaje);
				client->mensaje=mensaje;
				client->mEstado=0;
				write(client->fdCliente,"Mensaje enviado\n",16);

				while(contador==BUFFLEN){

				}

				//seccion critica: guardar lo producido
				buffer[entrada]=client;
				//final seccion critica
				write(client->fdCliente,"Mensaje Recibido\n",17);
				//guardado de variables
				entrada=entrada+1;
				contador=contador+1;


			}
			else if(read_size==0){

				printf("Cliente desconectado");
        		fflush(stdout);
        		cont=cont-1;
        		free(client);
        		close(client->fdCliente);
        		break;
        		exit(0);

				
			}
			else{
				write(client->fdCliente,"ERROR AL LEER\n",14);
			}

		}
		else{

			write(client->fdCliente,"espere...\n",10);
			while(client->mEstado==0){

			}
		}

	}	
}


void * servidorConsumidor(void *ptr){

	cliente* client;

	while(1){

		if(contador==0){

			printf("buffer vacio: No existe nada a consumir\n");
			while(contador==0){				// se puede consumir?
			//esta vacio no puede consumir
			}
		}
		

		printf("consumiendo\n");

		//inicio seccion critica
		client=buffer[salida];
		//final secion critica

		salida=(salida+1)%BUFFLEN;
		contador=contador-1;
		printf("procesando peticion cliente:%d\n",client->numCliente);
		procesarMensaje(client);


	}

}


int nivelar(char *cadena){

	int i;
	for (i = 0; *(cadena+i)!='\n'; ++i)
	{
		
		if(*(cadena+i+1)=='\n'){


		*(cadena+i)=*(cadena+i+2);
		break;
		}	
		/* code */
	}

	return 1;

}


/*
int separarCadenaEspacios(char*cadena){

 	int cont=0;
 	int i;
 	char*cadena;
 	for ( i = 0; i < strlen(cadena); ++i)
 	{
 		if(*(cadena+i)==' '){
 			cont=cont+1;
 		}
 		
 	}


 	for ( i = 0; i < strlen(cadena); ++i)
 	{
 		
 		cadena=malloc(sizeof(char))
 		
 	}




}
*/

int procesarMensaje(cliente*client){

	int valor;

	char separador[] = " ";

    char *trozo = NULL;

    trozo = strtok( client->mensaje, separador);

    while( trozo != NULL ) {
        printf( "Trozo : %s \n", trozo);
        trozo = strtok( NULL, separador);
    }




	
	if(strcmp( client->mensaje, "USER" )==0){
		
	}

	else if(strcmp( client->mensaje, "nickname" )==0){

	}

	
	printf("Peticion del cliente #%d--mensaje:%s--de tamano:%d procesando en este momento...\n",client->numCliente,client->mensaje,strlen(client->mensaje));
	write(client->fdCliente,"comando en el servidor procesando...\n",37);



	client->mEstado=1;		// con esto ya puede entrar al read;
	
	return 1;
}

int main(int argc, char ** argv)
{
	int sock = -1;
	struct sockaddr_in address;
	int port;
	connection_t * connection;
	pthread_t threadProductor, threadConsumidor;
	cliente * client;
	/* check for command line arguments */
	if (argc != 2)
	{
		fprintf(stderr, "usage: %s port\n", argv[0]);
		return -1;
	}

	/* obtain port number */
	if (sscanf(argv[1], "%d", &port) <= 0)
	{
		fprintf(stderr, "%s: error: wrong parameter: port\n", argv[0]);
		return -2;
	}

	/* create socket */
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock <= 0)
	{
		fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
		return -3;
	}

	/* bind socket to port */
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
	{
		fprintf(stderr, "%s: error: cannot bind socket to port %d\n", argv[0], port);
		return -4;
	}

	/* listen on port */
	if (listen(sock, 5) < 0)
	{
		fprintf(stderr, "%s: error: cannot listen on port\n", argv[0]);
		return -5;
	}

	printf("%s: ready and listening\n", argv[0]);

	
	inicializarClientes();
	inicializarCanales();

	pthread_create(&threadConsumidor, 0, servidorConsumidor, NULL);
	
	while (1)
	{
		/* accept incoming connections */
		//connection = (connection_t *)malloc(sizeof(connection_t));
		//connection->sock = accept(sock, &connection->address, &connection->addr_len);

		if(cont<MaxClientes){



			client=(cliente*)malloc(sizeof(cliente));
			client->fdCliente=accept(sock,&client->address,&client->addr_len);


			
			if (client->fdCliente <= 0)
			{
				printf("No se pudo establecer coneccion\n");
				free(client);
			}
			else
			{

				//CONEXXION ACEPTADA
				client->numCliente=cont;
				printf("CLIENTE #%d\n",client->numCliente);
				pthread_create(&threadProductor, 0, atencionClienteProductor, (void *)client);
				clientes[cont]=client;
				cont=cont+1;
				//pthread_create(&thread, 0, process, (void *)connection);
				//pthread_detach(thread);

			}
		}

	}
	
	return 0;
}




