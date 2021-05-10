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

struct UserNode {
	User *usr;
	struct UserNode* next;
};

struct ProductNode {
	Product *product;
	struct ProductNode* next;
};

struct OrderNode {
	Order *order;
	struct OrderNode* next;
};

struct UserList {
	int size;
	struct UserNode* head;
	struct UserNode* tail;
};

struct ProductList {
	int size;
	struct ProductNode* head;
	struct ProductNode* tail;
};

struct OrderList {
	int size;
	struct OrderNode* head;
	struct OrderNode* tail;
};

struct UserList users;
struct ProductList products;
struct OrderList orders;


short SocketCreate(void)
{
    short hSocket;
    printf("Create the socket\n");
    hSocket = socket(AF_INET, SOCK_STREAM, 0);
    return hSocket;
}

// Insert user to list
void insertUser(User* tmpUsr) {
	struct UserNode* userNode;
	userNode = (struct UserNode*) malloc(sizeof(struct UserNode));
    User* newUser = (User *) malloc(sizeof(User));
    memcpy(newUser, tmpUsr, sizeof(User));

	userNode->usr = newUser;
	userNode->next = NULL;

	if(users.head == NULL) {
		users.head = userNode;
		users.tail = userNode;
	}
	else {
		users.tail->next = userNode;
		users.tail = users.tail->next;
	}
	users.size++;
}

void insertOrder(Order *tmpOrder) {
	struct OrderNode* orderNode;
	orderNode = (struct OrderNode*) malloc(sizeof(struct OrderNode));
    Order* newOrder = (Order *) malloc(sizeof(Order));
    memcpy(newOrder, tmpOrder, sizeof(Order));

	orderNode->usr = newOrder;
	orderNode->next = NULL;

	if(orders.head == NULL) {
		orders.head = orderNode;
		orders.tail = orderNode;
	}
	else {
		orders.tail->next = orderNode;
		orders.tail = orders.tail->next;
	}
	orders.size++;
}

void readOrders() {
    FILE *file;
    char* fileName = "orders.txt";
    Order* order = (Order*) malloc(sizeof(Order));

    file = fopen(fileName, "r+");
	if(file == NULL) {
        perror("Error in fopen\n");
        exit(EXIT_FAILURE);
	}
    orders.size = 0;
    #if DEBUG == 1
    printf("File opened sucessfully\n");
    #endif
    while(!feof(file)) {
        memset(order->oid, '\0', sizeof(order->username));
        memset(order->pid, '\0', sizeof(order->username));
        fscanf(file, "%s %s %d", order->username, order->password, order->qty);
        insertOrder(order);
    }
    if(fclose(file) != 0) {
        perror("Error in fclose\n");
        exit(EXIT_FAILURE);
    }
}

void readConfig() {
    FILE *file;
    char* fileName = "config.txt";
    User* usr = (User*) malloc(sizeof(User));


    file = fopen(fileName, "r+");
	if(file == NULL) {
        perror("Error in fopen\n");
        exit(EXIT_FAILURE);
	}
    users.size = 0;
    #if DEBUG == 1
    printf("File opened sucessfully\n");
    #endif
    while(!feof(file)) {
        memset(usr->username, '\0', sizeof(usr->username));
        memset(usr->password , '\0', sizeof(usr->password));
        fscanf(file, "%s %s", usr->username, usr->password);
        insertUser(usr);
    }
    if(fclose(file) != 0) {
        perror("Error in fclose\n");
        exit(EXIT_FAILURE);
    }
}

void readConfig() {
    FILE *file;
    char* fileName = "config.txt";
    User* usr = (User*) malloc(sizeof(User));


    file = fopen(fileName, "r+");
	if(file == NULL) {
        perror("Error in fopen\n");
        exit(EXIT_FAILURE);
	}
    users.size = 0;
    #if DEBUG == 1
    printf("File opened sucessfully\n");
    #endif
    while(!feof(file)) {
        memset(usr->username, '\0', sizeof(usr->username));
        memset(usr->password , '\0', sizeof(usr->password));
        fscanf(file, "%s %s", usr->username, usr->password);
        insertUser(usr);
    }
    if(fclose(file) != 0) {
        perror("Error in fclose\n");
        exit(EXIT_FAILURE);
    }
}

void printUsers() {
	struct UserNode *curr = users.head;
	while(curr != NULL) {
        printf("%s %s\n", curr->usr->username, curr->usr->password);
        curr = curr->next;
	}
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

    // Read config file
    readConfig();
    printUsers();

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
            memset(client_message, '\0', sizeof client_message);
            memset(message, '\0', sizeof message);
            sleep(1);
        }
        printf("Closing socket...\n");
        close(sock);
    }
    return 0;
}
