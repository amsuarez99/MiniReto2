#include<stdio.h>
#include <stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<syslog.h>

#define DEBUG 1
#define MAX 50
#define PORT 8080

static void daemonize()
{
    pid_t pid;
    
    /* Fork off the parent process */
    pid = fork();
    
    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);
    
     /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);
    
    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);
    
    /* Catch, ignore and handle signals */
    /*TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    
    /* Fork off for the second time*/
    pid = fork();
    
    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);
    
    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);
    
    /* Set new file permissions */
    umask(0);
    
    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/home/nestor/Documents");
    
    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }
    
    /* Open the log file */
    openlog ("SERVER", LOG_PID, LOG_DAEMON);
}

typedef struct{
    char username[MAX];
    char password[MAX];
} User;
/*
struct UserNode{
	User user;
	UserNode* next;	  
};
struct UserList{
	int size;
	struct UserNode* head;
	struct UserNode* tail;
};
*/
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

void readFile(){

    FILE *ptr;
    
    ptr = fopen("config.txt", "r");
    
    if(ptr == NULL){
    	printf("No config file found \n");
    	exit(EXIT_FAILURE);
    }
    else{
    	syslog(LOG_NOTICE, "Archivo Abierto");	
    }
}

int main(int argc, char *argv[])
{
	daemonize();
	
	while(1){
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
    syslog(LOG_NOTICE, "Socket created");

    //Bind
    if( BindCreatedSocket(socket_desc) < 0) {
        //print the error message
        perror("Bind failed.\n");
        return 1;
    }
    printf("Bind done\n");

    //Listen
    listen(socket_desc, 3);
    
	readFile();
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
/*
            #if DEBUG == 1
            printf("%s\n", usr.username);
            #endif
*/
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
    }
    syslog(LOG_NOTICE, "Server Terminated.");
    closelog();
    
    
    return EXIT_SUCCESS;
}
