#include<stdio.h>
#include <stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>

#define DEBUG 1
#define MAX 50
#define PORT 8080

typedef struct{
    char username[MAX];
    char password[MAX];
} User;

short SocketCreate(void)
{
    short hSocket;
    printf("Create the socket\n");
    hSocket = socket(AF_INET, SOCK_STREAM, 0);
    return hSocket;
}

int BindCreatedSocket(int hSocket)
{
    int iRetval=-1;
    struct sockaddr_in  remote= {0};
    /* Internet address family */
    remote.sin_family = AF_INET;
    /* Any incoming interface */
    remote.sin_addr.s_addr = htonl(INADDR_ANY);
    remote.sin_port = htons(PORT); /* Local port */
    iRetval = bind(hSocket,(struct sockaddr *)&remote,sizeof(remote));
    return iRetval;
}

User* castToUser(unsigned char* buffer, User usr) {
    return (User *) buffer;
}

int main(int argc, char *argv[])
{
    int socket_desc, sock, clientLen, read_size;

    struct sockaddr_in client;
    char client_message[2000]= {0};
    char message[100] = {0};
    const char *pMessage = "marcelo";
    //Create socket
    socket_desc = SocketCreate();
    if (socket_desc == -1) {
        perror("Could not create socket\n");
        exit(EXIT_FAILURE);
    }
    printf("Socket created\n");

    //Bind
    if( BindCreatedSocket(socket_desc) < 0) {
        //print the error message
        perror("Bind failed.\n");
        return 1;
    }
    printf("Bind done\n");

    //Listen
    listen(socket_desc, 3);

    //Accept an incoming connection
    while(1)
    {
        printf("Waiting for incoming connections...\n");
        clientLen = sizeof(struct sockaddr_in);

        //accept connection from an incoming client
        sock = accept(socket_desc,(struct sockaddr *)&client,(socklen_t*)&clientLen);
        if (sock < 0) {
            perror("Accept failed");
            return 1;
        }
        printf("Connection accepted\n");

        //Receive a reply from the client
        while(recv(sock, client_message, sizeof(User), 0) > -1) {
            User usr;
            memcpy(&usr,client_message,sizeof(User));

            #if DEBUG == 1
            printf("%s\n", usr.username);
            #endif

            // Routine
            if(strcmp(pMessage,usr.username)==0)
            {
                strcpy(message,"Authenticated");
                send(sock, message, strlen(message), 0);
            } else {
                send(sock, message, strlen(message), 0);
                break;
            }
            // Send some data
            /* if( send(sock, message, strlen(message), 0) < 0) */
            /* { */
            /*     printf("Send failed"); */
            /*     return 1; */
            /* } */
            memset(client_message, '\0', sizeof client_message);
            memset(message, '\0', sizeof message);
            sleep(1);
        }
        printf("Closing socket...\n");
        close(sock);
    }
    return 0;
}
