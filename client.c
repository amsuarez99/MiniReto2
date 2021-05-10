#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define RESPONSE_SIZE 3000
#define REQUEST_SIZE 200
#define MAX 50
#define SERVER_PORT 8083

enum OPERATION {
    INSERT,
    SELECT,
    JOIN,
    DISCONNECT
};

typedef struct{
    char username[MAX];
    char password[MAX];
} User;

typedef struct{
    char table[MAX];
    char where[2];
    char cond[MAX];
} Select;

typedef struct {
    char pid[MAX];
    char pname[MAX];
    float price;
    char description[MAX];
} Product;

typedef struct {
    char oid[MAX];
    char pid[MAX];
    int qty;
} Order;

// return a user
User inputUser() {
    User temp;
    printf("Introduce tus credenciales:\n");
    printf("Usuario> ");
    scanf("%s", temp.username);
    printf("Contraseña> ");
    scanf("%s", temp.password);


    // Allocate memory for the pointers of the struct
    // User *u = malloc(sizeof(struct User));

    // // Allocate memory for contents of pointers.
    // p->username = malloc(strlen(username) + 1);
    // p->password = malloc(strlen(password) + 1);

    // strcpy(u->name, username);
    // strcpy(u->password, password);

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
int SocketReceive(int hSocket,char* Rsp, int RvcSize)
{
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 20;  /* 20 Secs Timeout */
    tv.tv_usec = 0;
    if(setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof(tv)) < 0) {
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

void toLower(char* c, size_t l) {
    for(int i = 0; i < l; i++) {
        c[i] = tolower(c[i]);
    }
}

void validateInsert(int socket, const struct sockaddr *dest, socklen_t dlen) {
    char table[MAX];
    scanf(" %s", table);
    toLower(table, sizeof(table));
    unsigned char response[RESPONSE_SIZE] = {0};
    SocketSend(socket, table, sizeof(table));
    if(strcmp(table, "orders") == 0) {
        Order otemp;
        unsigned char buffer[sizeof(Order)];
        scanf(" %s %s %d", otemp.oid, otemp.pid, &otemp.qty);
        memcpy(buffer, &otemp, sizeof(buffer));
        sendto(socket, buffer, sizeof(buffer), 0, dest, dlen);
    } else if(strcmp(table, "products") == 0) {
        Product ptemp;
        unsigned char buffer[sizeof(Product)];
        scanf(" %s %s %f %[^\n]", ptemp.pid, ptemp.pname, &ptemp.price, ptemp.description);
        memcpy(buffer, &ptemp, sizeof(buffer));
        SocketSend(socket, buffer, sizeof(buffer));
    }
}

//main driver program
int main(int argc, char *argv[])
{
    char *tabla, *where, *cond;
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

    
    enum OPERATION op;
    char params[REQUEST_SIZE];
    char table[MAX];
    memset(params, '\0', sizeof(params));
    // Authenticated -- Run program
    do {
        printMenu();
        char opt;
        enum OPERATION op;

        fflush(stdout);
        scanf(" %c", &opt);
        op = opt - '0';
        // Send option to server
        SocketSend(hSocket, &opt, 1);
        switch(op) {
            case INSERT: 
                printf("FORMATO [TABLE] [ATR1] [ATR2] [ATR3]\n");
                fflush(stdout);
                validateInsert(hSocket, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
                break;
            case SELECT:
                printf("FORMATO [TABLE] [ATR1] [OPERATOR] [VALUE]\n");
                fflush(stdout);
                scanf(" %[^\n]", params);
                // validateSelect(params);
                break;
            case JOIN:
                printf("FORMATO [TABLE] [TABLE]\n");
                fflush(stdout);
                scanf(" %[^\n]", params);
                // validateJoin(params);
                break;
            case DISCONNECT:
                printf("Disconnecting from DB...\n");
                close(hSocket);
                shutdown(hSocket,0);
                shutdown(hSocket,1);
                shutdown(hSocket,2);
                exit(EXIT_SUCCESS);
                break;
            default:
                printf("Invalid option\n");
                fflush(stdout);
                break;
        }
        // Send params to server
        if(op != DISCONNECT) {
            SocketSend(hSocket, params, strlen(params));
            memset(params, '\0', sizeof(params));
        }

    } while(1);
    return 0;
}
