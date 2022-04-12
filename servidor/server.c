#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
void* comunicacion (void*);
int main(int argc, char *argv[]){
    /** Descriptor el cuál tendra 0 si fue creado satisfactoriamente o 0 en caso contrario */
    int fd_socket;

    /** Estructura el cual se llenara con la informacion del servidor */
    struct sockaddr_in servidor;
    /** Estructura el cual se llenara con la informacion del servidor */
    struct sockaddr_in cliente;

    // DECLARACION DE LAS VARIABLES UTILIZADAS DENTRO LA RECEPCION DE LA INFORMACION POR PARTE DEL CLIENTE
    /** Variable para guardar canal de conexion */
    int *id_canal;
    /** Variable para almacenar el tamano de la estructura del cliente*/
    socklen_t tamano_cliente = sizeof(struct sockaddr_in);
    /** Variable para guardar la cantidad de bytes enviados */
    int bytes_enviados;
    /** Variable para guardar la cantidad de bytes recibidos */
    int bytes_recibidos;
    int BUFFER_SIZE = 250;
    /** Variable para guardar la informacion recibida y enviada por el socket */
    char bufferEnv[BUFFER_SIZE];
    char bufferRec[BUFFER_SIZE];

    if(argc != 3){
        printf("Favor de ingresa el puerto y direcion IP\n");
        exit(-1);
    }

    /*------------------------------------*/
    /*-----INICIALIZACION DEL SOCKET -----*/
    /*------------------------------------*/
    if((fd_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("Error al crear el socket\n");
        exit(-1);
    }

    // Inicializacion de la familia de la direccion
    servidor.sin_family = AF_INET;
    // Inicializacion del puerto
    int puerto = atoi(argv[1]);
    servidor.sin_port = htons(puerto);
    // Inicializacion de la direccion del socket
    inet_aton(argv[2],&servidor.sin_addr);

    /*------------------------------------*/
    /*------ VINCULACION DEL SOCKET ------*/
    /*------------------------------------*/
    if (bind(fd_socket, (struct sockaddr*) &servidor, sizeof(struct sockaddr)) == -1){
        printf("Error al vincular el socket\n");
        exit(-1);
    }

    /*--------------------------------------------*/
    /*------ LISTEN DEL SOCKET DEL SERVIDOR ------*/
    /*--------------------------------------------*/
    if(listen(fd_socket, 4)){
        printf("Error en el listen()\n");
        exit(-1);
    }

    /* Ciclo infinito para siempre estar atento a recibir informacion del cliente */
    while(1) {

        /*---------------------------------------------------*/
        /*------ ACEPTACION DE UNA CONEXION DEL SOCKET ------*/
        /*---------------------------------------------------*/
        id_canal = (int*)malloc(sizeof(int)); //Se esta generando una variable en memoria dinamica
        printf("En espera de una conexion\n");
        if((*id_canal = accept(fd_socket, (struct sockaddr *)&cliente, &tamano_cliente)) == -1){
            printf("Error en el accept()\n");
            exit(-1);
        }
         printf("Conexion aceptada\n");
        pthread_t hilo;
        if(pthread_create(&hilo,NULL,comunicacion,(void*)id_canal) == -1){
            printf("No se pudo crear el hilo\n");
            exit(-1);
        }
    }
}

void *comunicacion(void* idCanal){    
    int *id_canal = (int*) idCanal;
    printf("id Canal: %d",*id_canal);
    /** Variable para guardar la cantidad de bytes enviados */
    int bytes_enviados;
    /** Variable para guardar la cantidad de bytes recibidos */
    int bytes_recibidos;
    int BUFFER_SIZE = 250;
    /** Variable para guardar la informacion recibida y enviada por el socket */
    char bufferEnv[BUFFER_SIZE];
    char bufferRec[BUFFER_SIZE];
        if((bytes_recibidos = recv(*id_canal, bufferRec, sizeof(bufferRec), 0))==-1){
            printf("Error en el recv()\n");
            exit(-1);
        }
        bufferRec[bytes_recibidos] = '\0';

        printf("El nombre del archivo es %s\n", bufferRec);

        FILE *archivo = fopen(bufferRec, "ab");
        if (archivo == NULL) {
            printf("No se pudo construir al archivo\n");
            strcpy(bufferEnv, "ERROR");
            if ((bytes_enviados= send(*id_canal, bufferEnv, sizeof(bufferEnv), 0)) == -1) {
                printf("Error en el send()\n");
                exit(-1);
            }
            exit(1);
        } else {
            strcpy(bufferEnv, "ACK");
            if ((bytes_enviados= send(*id_canal, bufferEnv, sizeof(bufferEnv), 0)) == -1) {
                printf("Error en el send()\n");
                fclose(archivo);
                exit(-1);
            }
        }
        int contador =0;
        while (1){
            /*------ RECEPCION DEL MENSAJE DEL SOCKET ------*/
            if((bytes_recibidos = recv(*id_canal, bufferRec, sizeof(bufferRec), 0))==-1){
                printf("Error en el recv()\n");
                fclose(archivo);
                exit(-1);
            }
            //bufferRec[bytes_recibidos] = '\0';
            
             //   contador++;
             //   printf("Contador: %d, BytesRec: %d, BufferRec: %s\n",contador,bytes_recibidos,bufferRec);
            if (strncmp(bufferRec, ".....",5) == 0) {
                printf("Se ha recorrido todo el archivo\n");
                close(*id_canal);
                fclose(archivo);
                break;
            }else{
                fwrite(bufferRec,sizeof(char),bytes_recibidos,archivo);
            }
            /*------ ENVIO DEL MENSAJE EN EL SOCKET ------*/
            strcpy(bufferEnv, "ACK");
            if ((bytes_enviados= send(*id_canal, bufferEnv, sizeof(bufferEnv), 0)) == -1) {
                printf("Error en el send()\n");
                fclose(archivo);
                exit(-1);
            }
            memset(bufferEnv,0,BUFFER_SIZE);
            memset(bufferRec,0,BUFFER_SIZE);
        }
        free(id_canal);
}