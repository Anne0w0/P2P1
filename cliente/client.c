#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
//función para la comunicación con el socket creado
int comunicacion(int puerto, char* ip, char *arch);
const char** menu();

int main(int argc, char *argv[]){

    if(argc != 3){
        printf("Favor de ingresa el puerto, direcion IP\n");
        exit(-1);
        
    }

    // Inicializacion del puerto
    //int puerto = atoi(argv[1]);
    //Mostramos el menu que nos regresara los nombres de los archivos que buscamos
    const char** archivos = *archivos[100];

    archivos = menu();
    int i = 0;
    for (int i = 0; archivos != "0000"; i++){
        printf("%s\n", archivos[i]);
    }

    /*Hacemos la comunicacion
    if(comunicacion(puerto, argv[2],argv[3]) != 0){
        printf("Error fatal");
        exit(-1);
    }
   
    printf("Envio exitoso\n");*/
    return 0;
}

const char** menu(){
    int opc;
    int ban = 1;
    const char* archivos[] ={"archivo1", "archivo2", "archivo3", "archivo4", "archivo5","0000"};
    printf("Que tipo de archivo es el que desea enviar?\n1 .txt\n 2 .pdf\n 3 .jpg \n4 .mp3\n");
    scanf("%d", &opc);
    while (ban)
    {
      switch (opc)
        {
            //caso de .txt
            case 1:
                printf("busqueda de archivos .txt ");
                ban = 0;
                break;
            //caso de .pdf
            case 2:
                printf("busqueda de archivos .pdf ");
                ban = 0;
                break;
            //caso de .jpg
            case 3:
                printf("busqueda de archivos .jpg ");
                ban = 0;
                break;
            //caso de .mp3
            case 4:
                printf("busqueda de archivos .mp3 ");
                ban = 0;
                break;
            default:
                printf("elija una opcion valida");
                break;
        }
    }
    
    return &archivos;

}

int comunicacion(int puerto, char* ip, char *arch){
    // DECLARACION DE LAS VARIABLES UTILIZADAS DENTRO LA RECEPCION DE LA INFORMACION POR PARTE DEL CLIENTE
    /** Variable para guardar canal de conexion */
    int id_canal;
    /** Variable para almacenar el tamano de la estructura del cliente*/
    socklen_t tamano_cliente = sizeof(struct sockaddr_in);
    /** Variable para guardar la cantidad de bytes enviados */
    int bytes_enviados;
    /** Variable para guardar la cantidad de bytes recibidos */
    int bytes_recibidos;
    /** Variable para guardar el tamaño del buffer */
    int BUFFER_SIZE = 250;
    /** Variable para guardar la informacion recibida y enviada por el socket */
    char bufferEnv[BUFFER_SIZE];
    char bufferRec[BUFFER_SIZE];

    /** Descriptor el cuál tendra 0 si fue creado satisfactoriamente o 0 en caso contrario */
    int fd_socket;

    /** Estructura el cual se llenara con la informacion del servidor */
    struct sockaddr_in servidor;
    /** Estructura el cual se llenara con la informacion del servidor */
    struct sockaddr_in cliente;


    

    /*------------------------------------*/
    /*-----INICIALIZACION DEL SOCKET -----*/
    /*------------------------------------*/
    if((fd_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("Error al crear el socket\n");
        return -1;
    }

    // Inicializacion de la familia de la direccion
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(puerto);
    // Inicializacion de la direccion del socket
    inet_aton(ip,&servidor.sin_addr);

     FILE *archivo = fopen(arch, "rb"); // Apuntador para el manejo del archivo
    if (archivo == NULL) {
        printf("El archivo [%s] no existe\n", arch);
       return -1;
    }

    /*-----------------------------*/
    /*-----CONEXION AL SOCKET -----*/
    /*-----------------------------*/
    if(connect(fd_socket, (struct sockaddr*) &servidor, sizeof(struct sockaddr)) == -1){
        printf("Error al conectarse al socket\n");
        fclose(archivo);
        return -1;
    }
    printf("Conexion establecida con el servidor\n");

    if((bytes_enviados = send(fd_socket, arch, strlen(arch) ,0)) == -1){
        printf("Error al enviar el nombre del archivo\n");
        fclose(archivo);
       return -1;
    }


    if((bytes_recibidos = recv(fd_socket, bufferRec, sizeof(bufferRec), 0))==-1){
        printf("Error en el recv()\n");
        fclose(archivo);
        return -1;
    }
    bufferRec[bytes_recibidos] = '\0';
    if (strcmp(bufferRec, "ACK") != 0) {
        printf("El servidor no pudo aceptar\n");
        close(fd_socket);
        fclose(archivo);
        return -1;
    }
    int contador=0;
    while (!feof(archivo)) {
        // LECTURA CONTENIDO ARCHIVO
        if (!fread(&bufferEnv,sizeof(char),BUFFER_SIZE, archivo)) {
            break;
        }
        /*----- ENVIO DE INFORMACION  AL SOCKET -----*/
        if((bytes_enviados = send(fd_socket, bufferEnv, sizeof(bufferEnv) ,0)) == -1){
            printf("Error al enviar el mensaje\n");
            fclose(archivo);
            return -1;
        }
        //contador++;
        //printf("Contador: %d, Bytes enviados: %d ,Contenido BufferEnv: %s\n",contador,bytes_enviados,bufferEnv);
        /*------ RECEPCION DEL MENSAJE DEL SOCKET ------*/
        if((bytes_recibidos = recv(fd_socket, bufferRec, sizeof(bufferRec), 0))==-1){
            printf("Error en el recv()\n");
            fclose(archivo);
            return -1;
        }
        bufferRec[bytes_recibidos] = '\0';
        printf("%s\n", bufferRec);
        if (strcmp(bufferRec, "ACK") != 0) {
            printf("Error al rercibir el contenido del archivo\n");
            close(fd_socket);
            fclose(archivo);
            exit(0);
        }
        memset(bufferEnv,0,BUFFER_SIZE);
        memset(bufferRec,0,BUFFER_SIZE);
    }
    strcpy(bufferEnv,".....");
    if((bytes_enviados = send(fd_socket, bufferEnv, sizeof(bufferEnv) ,0)) == -1){
        printf("Error al enviar el mensaje\n");
        fclose(archivo);
        return -1;
    }
    printf(".....\n");
    fclose(archivo);

    printf("Se ha terminado de enviar el archivo\n");

    return 0;
}