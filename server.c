#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

#define DEBUG 1
#define MAX 50
#define PORT 8085
#define RESPONSE_SIZE 3000
#define REQUEST_SIZE 200

FILE *fOrders = NULL, *fProducts = NULL, *fConfig = NULL;

static void signalHandler(int signo) {
    if(signo == SIGTERM) {
        if (fclose(fProducts) != 0)
    	{
             perror("Error in fclose\n");
             syslog(LOG_NOTICE, "Error in fclose");
             exit(EXIT_FAILURE);
    	}
    	if (fclose(fOrders) != 0)
    	{
             perror("Error in fclose\n");
             syslog(LOG_NOTICE, "Error in fclose");
             exit(EXIT_FAILURE);
    	}
    	if (fclose(fConfig) != 0)
    	{
             perror("Error in fclose\n");
             syslog(LOG_NOTICE, "Error in fclose");
             exit(EXIT_FAILURE);
    	}
    }
}

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
    signal(SIGTERM, signalHandler);

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
    chdir("Users/amsua/Documents/Tec/Feb-2021/progra/sockets/");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
    {
        close(x);
    }

    /* Open the log file */
    openlog("SERVER", LOG_PID, LOG_DAEMON);
}

enum OPERATION
{
    INSERT,
    SELECT,
    JOIN,
    DISCONNECT
};

typedef struct
{
    char username[MAX];
    char password[MAX];
} User;

typedef struct
{
    char pid[MAX];
    char pname[MAX];
    float price;
    char description[MAX];
} Product;

typedef struct
{
    char oid[MAX];
    char pid[MAX];
    int qty;
} Order;

typedef struct
{
    char table[MAX];
    char operator[3];
    char value[MAX];
    char attribute[MAX];
} Query;

struct UserNode
{
    User *usr;
    struct UserNode *next;
};

struct ProductNode
{
    Product *product;
    struct ProductNode *next;
};

struct OrderNode
{
    Order *order;
    struct OrderNode *next;
};

struct UserList
{
    int size;
    struct UserNode *head;
    struct UserNode *tail;
};

struct ProductList
{
    int size;
    struct ProductNode *head;
    struct ProductNode *tail;
};

struct OrderList
{
    int size;
    struct OrderNode *head;
    struct OrderNode *tail;
};

struct UserList users;
struct ProductList products;
struct OrderList orders;

short SocketCreate(void)
{
    short hSocket;
    printf("Create the socket\n");
    syslog(LOG_NOTICE, "Create the socket");
    hSocket = socket(AF_INET, SOCK_STREAM, 0);
    return hSocket;
}

// Insert user to list
void insertUser(User *tmpUsr)
{
    struct UserNode *userNode;
    userNode = (struct UserNode *)malloc(sizeof(struct UserNode));
    User *newUser = (User *)malloc(sizeof(User));
    memcpy(newUser, tmpUsr, sizeof(User));

    userNode->usr = newUser;
    userNode->next = NULL;

    if (users.head == NULL)
    {
        users.head = userNode;
        users.tail = userNode;
    }
    else
    {
        users.tail->next = userNode;
        users.tail = users.tail->next;
    }
    users.size++;
}
void insertOrder(Order *tmpOrder)
{
    struct OrderNode *orderNode;
    orderNode = (struct OrderNode *)malloc(sizeof(struct OrderNode));
    Order *newOrder = (Order *)malloc(sizeof(Order));
    memcpy(newOrder, tmpOrder, sizeof(Order));

    orderNode->order = newOrder;
    orderNode->next = NULL;

    if (orders.head == NULL)
    {
        orders.head = orderNode;
        orders.tail = orderNode;
    }
    else
    {
        orders.tail->next = orderNode;
        orders.tail = orders.tail->next;
    }
    orders.size++;
}

void insertProduct(Product *tmpProduct)
{
    struct ProductNode *productNode;
    productNode = (struct ProductNode *)malloc(sizeof(struct ProductNode));
    Product *newProduct = (Product *)malloc(sizeof(Product));
    memcpy(newProduct, tmpProduct, sizeof(Product));

    productNode->product = newProduct;
    productNode->next = NULL;

    if (products.head == NULL)
    {
        products.head = productNode;
        products.tail = productNode;
    }
    else
    {
        products.tail->next = productNode;
        products.tail = products.tail->next;
    }
    products.size++;
}

FILE* readOrders()
{
    FILE* file;
    char *fileName = "orders.txt";
    Order *order = (Order *)malloc(sizeof(Order));

    file = fopen(fileName, "r+");
    if (file == NULL)
    {
        perror("Error in fopen\n");
        exit(EXIT_FAILURE);
    }
    orders.size = 0;
#if DEBUG == 1
    printf("File opened sucessfully\n");
#endif
    while (!feof(file))
    {
        memset(order->oid, '\0', sizeof(order->oid));
        memset(order->pid, '\0', sizeof(order->pid));
        fscanf(file, "%s %s %d", order->oid, order->pid, &order->qty);
        insertOrder(order);
    }
    return file;
}

FILE* readProducts()
{
    FILE *file;
    char *fileName = "products.txt";
    Product *product = (Product *)malloc(sizeof(Product));

    file = fopen(fileName, "r+");
    if (file == NULL)
    {
        perror("Error in fopen\n");
        exit(EXIT_FAILURE);
    }
    products.size = 0;
#if DEBUG == 1
    printf("File opened sucessfully\n");
#endif
    while (!feof(file))
    {
        memset(product->pid, '\0', sizeof(product->pid));
        memset(product->pname, '\0', sizeof(product->pname));
        memset(product->description, '\0', sizeof(product->description));
        fscanf(file, "%s %s %f %[^\n]", product->pid, product->pname, &product->price, product->description);
        insertProduct(product);
    }
    return file;
}

FILE* readConfig()
{
    FILE *file;
    char *fileName = "config.txt";
    User *usr = (User *)malloc(sizeof(User));

    file = fopen(fileName, "r+");
    if (file == NULL)
    {
        perror("Error in fopen\n");
        syslog(LOG_NOTICE, "Error in fopen");
        exit(EXIT_FAILURE);
    }
    users.size = 0;
#if DEBUG == 1
    printf("File opened sucessfully\n");
    syslog(LOG_NOTICE, "Filed opened succesfully");
#endif
    while (!feof(file))
    {
        memset(usr->username, '\0', sizeof(usr->username));
        memset(usr->password, '\0', sizeof(usr->password));
        fscanf(file, "%s %s", usr->username, usr->password);
        insertUser(usr);
    }
    if (fclose(file) != 0)
    {
        perror("Error in fclose\n");
        syslog(LOG_NOTICE, "Error in fclose");
        exit(EXIT_FAILURE);
    }
    return file;
}

void printUsers()
{
    struct UserNode *curr = users.head;
    while (curr != NULL)
    {
        printf("%s %s\n", curr->usr->username, curr->usr->password);
        curr = curr->next;
    }
}

void printProducts()
{
    struct ProductNode *curr = products.head;
    while (curr != NULL)
    {
        printf("%s %s %f %s\n", curr->product->pid, curr->product->pname, curr->product->price, curr->product->description);
        curr = curr->next;
    }
}

void printOrders()
{
    struct OrderNode *curr = orders.head;
    while (curr != NULL)
    {
        printf("%s %s %d\n", curr->order->oid, curr->order->pid, curr->order->qty);
        curr = curr->next;
    }
}

int BindCreatedSocket(int hSocket)
{
    int iRetval = -1;
    struct sockaddr_in remote = {0};
    /* Internet address family */
    remote.sin_family = AF_INET;
    /* Any incoming interface */
    remote.sin_addr.s_addr = htonl(INADDR_ANY);
    remote.sin_port = htons(PORT); /* Local port */
    iRetval = bind(hSocket, (struct sockaddr *)&remote, sizeof(remote));
    return iRetval;
}

//validar usuarios
int validate(User usr)
{
    struct UserNode *curr = users.head;
    while (curr != NULL)
    {
        if (strcmp(curr->usr->username, usr.username) == 0 && strcmp(curr->usr->password, usr.password) == 0)
        {
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}

void query(char *message, Query q)
{
    char *aux = (char *)malloc(RESPONSE_SIZE);
    aux[0] = '\0';
    if (strcmp(q.attribute, "*") == 0)
    {
        if (strcmp(q.table, "orders") == 0)
        {
            struct OrderNode *curr = orders.head;
            while (curr != NULL)
            {
                sprintf(aux, "%s %s %d\n", curr->order->oid, curr->order->pid, curr->order->qty);
                strcat(message, aux);
                memset(aux, '\0', RESPONSE_SIZE);
                curr = curr->next;
            }
        }
        else if (strcmp(q.table, "products") == 0)
        {
            struct ProductNode *curr = products.head;
            while (curr != NULL)
            {
                sprintf(aux, "%s %s %f ", curr->product->pid, curr->product->pname, curr->product->price);
                strcat(aux, curr->product->description);
                strcat(aux, "\n");
                strcat(message, aux);
                memset(aux, '\0', RESPONSE_SIZE);
                curr = curr->next;
            }
        }
    }
    else
    {
        if (strcmp(q.table, "orders") == 0)
        {
            struct OrderNode *curr = orders.head;
            while (curr != NULL)
            {
                if(strcmp(q.attribute, "oid") == 0) {
                    if(strcmp(q.operator, "EQ") == 0) {
                        if(strcmp(curr->order->oid, q.value) == 0) {
                            sprintf(aux, "%s %s %d\n", curr->order->oid, curr->order->pid, curr->order->qty);
                        }
                    }
                } else if(strcmp(q.attribute, "pid") == 0) {
                    if(strcmp(q.operator, "EQ") == 0) {
                        if(strcmp(curr->order->pid, q.value) == 0) {
                            sprintf(aux, "%s %s %d\n", curr->order->oid, curr->order->pid, curr->order->qty);
                        }
                    }
                } else if(strcmp(q.attribute, "qty") == 0) {
                    if(strcmp(q.operator, "LT") == 0 && curr->order->qty < atoi(q.value)) {
                        sprintf(aux, "%s %s %d\n", curr->order->oid, curr->order->pid, curr->order->qty);
                    } else if(strcmp(q.operator, "LTE") == 0 && curr->order->qty <= atoi(q.value)) {
                        sprintf(aux, "%s %s %d\n", curr->order->oid, curr->order->pid, curr->order->qty);
                    } else if(strcmp(q.operator, "GT") == 0 && curr->order->qty > atoi(q.value)) {
                        sprintf(aux, "%s %s %d\n", curr->order->oid, curr->order->pid, curr->order->qty);
                    } else if(strcmp(q.operator, "GTE") == 0 && curr->order->qty >= atoi(q.value)) {
                        sprintf(aux, "%s %s %d\n", curr->order->oid, curr->order->pid, curr->order->qty);
                    } else if(strcmp(q.operator, "EQ") == 0 && curr->order->qty == atoi(q.value)) {
                        sprintf(aux, "%s %s %d\n", curr->order->oid, curr->order->pid, curr->order->qty);
                    }
                }
                strcat(message, aux);
                memset(aux, '\0', RESPONSE_SIZE);
                curr = curr->next;
            }
        }
        else if (strcmp(q.table, "products") == 0)
        {
            struct ProductNode *curr = products.head;
            while (curr != NULL)
            {
                if(strcmp(q.attribute, "pid") == 0) {
                    if(strcmp(q.operator, "EQ") == 0) {
                        if(strcmp(curr->product->pid, q.value) == 0) {
                            sprintf(aux, "%s %s %f %s\n", curr->product->pid, curr->product->pname, curr->product->price, curr->product->description);
                        }
                    }
                } else if(strcmp(q.attribute, "pname") == 0) {
                    if(strcmp(q.operator, "EQ") == 0) {
                        if(strcmp(curr->product->pname, q.value) == 0) {
                            sprintf(aux, "%s %s %f %s\n", curr->product->pid, curr->product->pname, curr->product->price, curr->product->description);
                        }
                    }
                } else if(strcmp(q.attribute, "description") == 0) {
                    if(strcmp(q.operator, "EQ") == 0) {
                        if(strcmp(curr->product->description, q.value) == 0) {
                            sprintf(aux, "%s %s %f %s\n", curr->product->pid, curr->product->pname, curr->product->price, curr->product->description);
                        }
                    }
                } else if(strcmp(q.attribute, "price") == 0) {
                    if(strcmp(q.operator, "LT") == 0 && curr->product->price < atof(q.value)) {
                        sprintf(aux, "%s %s %f %s\n", curr->product->pid, curr->product->pname, curr->product->price, curr->product->description);
                    } else if(strcmp(q.operator, "LTE") == 0 && curr->product->price <= atof(q.value)) {
                        sprintf(aux, "%s %s %f %s\n", curr->product->pid, curr->product->pname, curr->product->price, curr->product->description);
                    } else if(strcmp(q.operator, "GT") == 0 && curr->product->price > atof(q.value)) {
                        sprintf(aux, "%s %s %f %s\n", curr->product->pid, curr->product->pname, curr->product->price, curr->product->description);
                    } else if(strcmp(q.operator, "GTE") == 0 && curr->product->price >= atof(q.value)) {
                        sprintf(aux, "%s %s %f %s\n", curr->product->pid, curr->product->pname, curr->product->price, curr->product->description);
                    } else if(strcmp(q.operator, "EQ") == 0 && curr->product->price == atof(q.value)) {
                        sprintf(aux, "%s %s %f %s\n", curr->product->pid, curr->product->pname, curr->product->price, curr->product->description);
                    }
                }
                strcat(message, aux);
                memset(aux, '\0', RESPONSE_SIZE);
                curr = curr->next;
            }
        }
    }
    free(aux);
}

void join(char *message) {
    char *aux = (char *)malloc(RESPONSE_SIZE);
    struct ProductNode *pcurr = products.head;
    struct OrderNode *ocurr = orders.head;
    while (ocurr != NULL)
    {
        while (pcurr != NULL)
        {
            if (strcmp(ocurr->order->pid, pcurr->product->pid) == 0)
            {
                sprintf(aux, "%s %s %d %s %s %f %s\n",
                        ocurr->order->oid, ocurr->order->pid, ocurr->order->qty, pcurr->product->pid, pcurr->product->pname, pcurr->product->price, pcurr->product->description);
                strcat(message, aux);
                memset(aux, '\0', RESPONSE_SIZE);
            }
            pcurr = pcurr->next;
        }
        pcurr = products.head;
        ocurr = ocurr->next;
    }
    free(aux);
}

int main(int argc, char *argv[])
{
    daemonize();
    int socket_desc, sock, clientLen, read_size;

    struct sockaddr_in client;
    char client_message[2000] = {0};
    char message[RESPONSE_SIZE] = {0};

    //Create socket
    socket_desc = SocketCreate();
    if (socket_desc == -1)
    {
        perror("Could not create socket\n");
        syslog(LOG_NOTICE, "Could Not create socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket created\n");
    syslog(LOG_NOTICE, "Socket created");

    //Bind
    if (BindCreatedSocket(socket_desc) < 0)
    {
        //print the error message
        perror("Bind failed.\n");
        syslog(LOG_NOTICE, "Bind failed");
        return 1;
    }
    printf("Bind done\n");
    syslog(LOG_NOTICE, "Bind Done");
    //Listen
    listen(socket_desc, 3);

    // Read config file
    fConfig = readConfig();
    fProducts = readProducts();
    fOrders = readOrders();

    //Accept an incoming connection
    while (1)
    {
        printf("Waiting for incoming connections...\n");
        clientLen = sizeof(struct sockaddr_in);
        //accept connection from an incoming client
        sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&clientLen);
        if (sock < 0)
        {
            perror("Accept failed");
            return 1;
        }
        printf("Connection accepted\n");
        syslog(LOG_NOTICE, "Connection Accepted");

        //Validation
        recv(sock, client_message, sizeof(User), 0);
        User usr;
        memcpy(&usr, client_message, sizeof(User));

#if DEBUG == 1
        printf("%s\n", usr.username);
#endif

        // Routine
        if (validate(usr) == 1)
        {
            strcpy(message, "Authenticated");
            send(sock, message, strlen(message), 0);
        }
        else
        {
            strcpy(message, "Login failed");
            send(sock, message, strlen(message), 0);
            printf("Closing socket...\n");
            close(sock);
            continue;
        }

        printf("Waiting for response...\n");
        // Receive option from server
        while (recv(sock, client_message, 1, 0) > -1)
        {
            memset(message , '\0', RESPONSE_SIZE);
            char opt;
            enum OPERATION op;

            memcpy(&opt, client_message, 1);
            op = opt - '0';
            if (op == DISCONNECT)
                break;
            // recv(sock, client_message, strlen(client_message), 0);
            switch (op)
            {
            case INSERT:
                printf("Waiting for response...\n");
                recv(sock, client_message, sizeof(client_message), 0);
                if (strcmp(client_message, "orders") == 0)
                {
                    Order o;
                    printf("Waiting for response...\n");
                    recv(sock, client_message, sizeof(Order), 0);
                    memcpy(&o, client_message, sizeof(Order));
                    fprintf(fOrders, "\n%s %s %d", o.oid, o.pid, o.qty);
                    fflush(fOrders);
                    insertOrder(&o);
                    printOrders();
                }
                else if (strcmp(client_message, "products") == 0)
                {
                    Product p;
                    printf("Waiting for response...\n");
                    recv(sock, client_message, sizeof(Product), 0);
                    memcpy(&p, client_message, sizeof(Product));
                    fprintf(fProducts, "\n%s %s %f %s", p.pid, p.pname, p.price, p.description);
                    fflush(fProducts);
                    insertProduct(&p);
                    printProducts();
                }
                break;
            case SELECT:
            {
                Query q;
                memset(message, '\0', RESPONSE_SIZE);
                printf("Waiting for response...\n");
                recv(sock, client_message, sizeof(Query), 0);
                memcpy(&q, client_message, sizeof(Query));
                query(message, q);
                send(sock, message, strlen(message), 0);
                memset(message, '\0', RESPONSE_SIZE);
                break;
            }
            case JOIN:
                join(message);
                send(sock, message, strlen(message), 0);
                memset(message, '\0', RESPONSE_SIZE);
                break;
            default:
                //invalid
                printf("Waiting for response...\n");
                break;
            }

            memset(client_message, '\0', sizeof client_message);
            memset(message, '\0', RESPONSE_SIZE);
            sleep(1);
        }

        printf("Closing socket...\n");
        close(sock);
    }
    return 0;
}
