#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>

int numOfClients=0;
char clientsNames[64][64];
int clientsSock[64];
char messages[64][8*256];
int isMessage[64];


int getNumberOfProcesses(char* str, int* pids);
void *readFromClient(void *arg);
void sendUsers(int socket);
void sendMessage(int nrSocket, char* message);
int lookName(char *name);
void receiveMessages(int socket);
void error(char *msg);



int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno=0, ret, quit=0, pidsServers[2], i;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t thread;
    pid_t pidFork;
    char line[16];

    while((ret = getopt(argc, argv, "qp:")) != -1){
        switch(ret){
            case 'p':
                portno = atoi(optarg);
                break;
            case 'q':
                quit=1;
                break;
            case '?':
                printf("[brak obslugi parametru -%c]\n", optopt);
        }
    }

    char pidof[32] = "pidof ";
    strcat(pidof, argv[0]);
    FILE *cmd = popen(pidof, "r");
    fgets(line, 16, cmd);
    int numProcesses = getNumberOfProcesses(line, pidsServers);
    if(quit==1){
        if(numProcesses>1){
            for(i=0; i<numProcesses; ++i){
                if(pidsServers[i]!=getpid()){
                    kill(pidsServers[i], SIGINT);
                }
            }
            return 0;
        }
        else return 0;
    }
    if(portno == 0){
        printf("No number of port\n");
        exit(1);
    }
    if(numProcesses > 1){
        printf("Another instance of program is running\n");
        exit(1);
    }

    pidFork = fork();
    if(pidFork < 0){
        error("Error with forking");
    }
    else if(pidFork > 0){
        exit(0);
    }
    setsid();
    chdir("/");
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

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

int getNumberOfProcesses(char* str, int *pids){
    int i, num, j=0, k=0;
    char pid[8];
    if(strlen(str)>0)num = 1;
    else return 0;
    for(i=1; i<strlen(str); ++i){
        if(str[i]==' ' || str[i]=='\n'){
            if(str[i]==' ') ++num;
            str[i]='\0';
            strcpy(pid, &str[k]);
            pids[j]=atoi(pid);
            k=i+1;
            ++j;
        }
    }
    strcpy(pid, &str[k]);
    pids[j]=atoi(pid);
    return num;
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