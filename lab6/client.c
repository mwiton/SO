#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n, ret;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    struct in_addr addr;
    char name[64], *input=NULL, message[64*64];

    while((ret = getopt(argc, argv, "a:p:n:")) != -1){
        switch(ret){
            case 'a':
                if(inet_aton(optarg, &addr) == 0 ){
                    error("Invalid address");
                }
                break;
            case 'p':
                portno = atoi(optarg);
                break;
            case 'n':
                strcpy(name, optarg);
                break;
            case '?':
                printf("[brak obslugi parametru -%c]\n", optopt);
        }
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = addr.s_addr;
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    n = write(sockfd,name,strlen(name));
    if (n < 0)
        error("ERROR writing to socket");
    while(1){
        size_t len = 0;
        ssize_t length = getline(&input, &len, stdin);
        input[length-1]='\0';
        write(sockfd, input, len);
        read(sockfd, message, 64*64);
        printf("%s\n", message);
    }
    /*n = read(sockfd,buffer,255);
    if (n < 0)
        error("ERROR reading from socket");
    printf("%s\n",buffer);*/
    close(sockfd);
    return 0;
}