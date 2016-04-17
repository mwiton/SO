#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

int numOfClients=0;
char clientsNames[64][64];
int clientsSock[64];
char messages[64][8*256];
int isMessage[64];

void *readFromClient(void *arg);
void sendUsers(int socket);
void sendMessage(int nrSocket, char* message);
int lookName(char *name);
void receiveMessages(int socket);
void error(char *msg);



int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, pid, ret;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t thread;

    while((ret = getopt(argc, argv, "p:")) != -1){
        switch(ret){
            case 'p':
                portno = atoi(optarg);
                break;
            case '?':
                printf("[brak obslugi parametru -%c]\n", optopt);
        }
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    while (1) {
        newsockfd = accept(sockfd,
                           (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        clientsSock[numOfClients]=newsockfd;
        isMessage[numOfClients] = 0;
        ret=read(clientsSock[numOfClients], clientsNames[numOfClients], 64);
        if (ret < 0) error("ERROR reading from socket");
        int number = numOfClients;
        pthread_create(&thread, NULL, readFromClient, &number);
        numOfClients++;
    }
    close(sockfd);
    return 0;
}

void error(char *msg){
    perror(msg);
    exit(1);
}

void *readFromClient(void *arg){
    int* nrSocketp = (int*) arg;
    int nrSocket = *nrSocketp;
    printf("%d", nrSocket);
    int socket = clientsSock[nrSocket];
    int n;
    char message[256];
    while(1){
        n=read(socket, message, 256);
        if(n<0) pthread_exit(NULL);
        if(memcmp("users", message, 5) == 0){
            sendUsers(socket);
        }
        else if(memcmp("send", message, 4) == 0){
            sendMessage(nrSocket, &message[5]);
        }
        else if(memcmp("messages", message, 8) == 0){
            receiveMessages(nrSocket);
        }
        else if(memcmp("exit", message, 4) == 0) {
            strcpy(clientsNames[nrSocket], "\0");
            clientsSock[nrSocket]=0;
            close(socket);
            pthread_exit(NULL);
        }
        else{
            write(socket, "There is not such command", 26);
        }
    }
}

void sendUsers(int socket){
    char message[64*64];
    int i;
    strcpy(message, "");
    for (i=0; i<numOfClients; ++i){
        strcat(message, clientsNames[i]);
        if(clientsNames[i][0]!='\0')strcat(message, " ");
    }
    write(socket, message, 64*64);
}

void sendMessage(int nrSocket, char *message){
    size_t space = strcspn(message, " ");
    char strMessage[256];
    message[space]='\0';
    int nrSocket2 = lookName(message);
    if(nrSocket2 == -1){
        write(clientsSock[nrSocket], "This user is not connected to server", 37);
        return;
    }
    strcpy(strMessage, clientsNames[nrSocket]);
    strcat(strMessage, ": ");
    strcat(strMessage, &message[space+1]);
    if(isMessage[nrSocket2]!=1){
        isMessage[nrSocket2]=1;
        strcpy(messages[nrSocket2], strMessage);
    }
    else{
        strcat(messages[nrSocket2], "\n");
        strcat(messages[nrSocket2], strMessage);
    }
    write(clientsSock[nrSocket], "Message was sent", 17);
}

int lookName(char *name){
    int i;
    for(i=0; i<numOfClients; ++i){
        if(strcmp(name, clientsNames[i]) == 0){
            return i;
        }
    }
    return -1;
}

void receiveMessages(int nrSocket){
    if(isMessage[nrSocket] == 1){
        write(clientsSock[nrSocket], messages[nrSocket], 8*256);
        isMessage[nrSocket] = 0;
    }
    else write(clientsSock[nrSocket], "No messages", 12);
}