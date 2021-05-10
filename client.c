#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define RESPONSE_SIZE 200
#define MAX 50
#define SERVER_PORT 8080

enum OPTION {
    INSERT,
    QUERY,
    JOIN,
    DISCONNECT,
};

typedef struct{
    char username[MAX];
    char password[MAX];
} User;

// return a user
User inputUser() {
    User temp;
    printf("Introduce tus credenciales:\n");
    printf("Usuario> ");
    scanf("%s", temp.username);
    printf("Contraseña> ");
    scanf("%s", temp.password);
    return temp;
}

//Create a Socket for server communication
short SocketCreate(void)
{
    short hSocket;
    printf("Create the socket\n");
    // AF_INET (protocol IPV4), SOCKET_STREAM (TCP), 0 = ip protocol
    hSocket = socket(AF_INET, SOCK_STREAM, 0);
    return hSocket;
}

// Send the data to the server and set the timeout of 20 seconds
int SocketSend(int hSocket,char* Rqst,short lenRqst)
{
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 20;  /* 20 Secs Timeout */
    tv.tv_usec = 0;
    if(setsockopt(hSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv)) < 0)
    {
        printf("Time Out\n");
        return -1;
    }
    shortRetval = send(hSocket, Rqst, lenRqst, 0);
    return shortRetval;
}

//receive the data from the server
int SocketReceive(int hSocket,char* Rsp,short RvcSize)
{
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 20;  /* 20 Secs Timeout */
    tv.tv_usec = 0;
    if(setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof(tv)) < 0)
    {
        perror("Time out...\n");
        exit(EXIT_FAILURE);
    }
    shortRetval = recv(hSocket, Rsp, RvcSize, 0);
    return shortRetval;
}

int authenticate(int socket, const struct sockaddr *dest, socklen_t dlen, const User usr) {
    unsigned char buffer[sizeof(User)], response[RESPONSE_SIZE] = {0};
    memcpy(buffer, &usr, sizeof(buffer));
    sendto(socket, buffer, sizeof(buffer), 0, dest, dlen);
    SocketReceive(socket, response, RESPONSE_SIZE);
    #if DEBUG == 1
        printf("%s\n", response);
    #endif
    if(strcmp(response, "Authenticated") == 0) {
       return 1; 
    }
    return -1;
}

void printMenu() {
    printf("Selecciona la opción que deseas realizar:\n");
    printf("0) Insertar\n");
    printf("1) Query\n");
    printf("2) Join\n");
    printf("3) Salir\n");
    printf("?> ");
}

//main driver program
int main(int argc, char *argv[])
{
    int hSocket, read_size;

    // Remote address
    struct sockaddr_in serv_addr= {0};
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Local Host
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    // Create socket
    if((hSocket = SocketCreate()) == -1) {
        perror("No se pudo crear el socket...\n");
        exit(EXIT_FAILURE);
    }

    // Establish connection
    printf("Conectando con base de datos...\n");
    if(connect(hSocket,(struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("Error en la conexión.\n");
        exit(EXIT_FAILURE);
    }
    printf("Conexión exitosa\n");
    
    // Input User
    User usr = inputUser();
    printf("Verificando credenciales...\n");

    // Authenticate
    if(authenticate(hSocket, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in), usr) < 0) {
        perror("Credenciales inválidas!\n");
        exit(EXIT_FAILURE);
    }

    enum OPTION op;
    // Authenticated -- Run program
    do {
        printMenu();
        scanf("%d", &op);
        switch(op) {
            case INSERT: 
                break;
            case QUERY:
                break;
            case JOIN:
                break;
            default:
                printf("Invalid option\n");
                break;
        }

        // printf("Enter the Message: ");
        // gets(SendToServer);
        // //Send data to the server
        // SocketSend(hSocket, SendToServer, strlen(SendToServer));
        // //Received the data from the server
        // read_size = SocketReceive(hSocket, server_reply, 200);
        // printf("Server Response : %s\n\n",server_reply);
        fflush(stdin);
    } while(op != DISCONNECT);

    close(hSocket);
    shutdown(hSocket,0);
    shutdown(hSocket,1);
    shutdown(hSocket,2);
    return 0;
}
