#include <stdio.h>
#include <unistd.h>
#include <crypt.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>

#define SIZE_OF_BLOCK 4096
#define FALSE 0
#define TRUE 1

#define clear() printf("\033[H\033[J")
#define gotoxy(x, y) printf("\033[%d;%dH", x, y)

int file , isCorrectPassword = FALSE, sizeOfFile, readBytes=0;
char *hash, correctWord[32];
pthread_mutex_t fileMutex, printMutex;

void printProgressPercent(){
    float percent;
    percent = (float)readBytes/sizeOfFile*100;
    clear();
    gotoxy(0, 0);
    printf("Percent of read bytes: %f%%", percent);
    fflush(stdout);
}

void comparePasswordAndHash(char *password, char *hash){
    char* cryptedPassword;
    cryptedPassword = crypt(password, hash);
    if (strcmp(cryptedPassword, hash) == 0){
        printf("\n%s", password);
        isCorrectPassword = TRUE;
        strcpy(correctWord, password);
    }
}

void *threadFunction(void *arg){
    char data[SIZE_OF_BLOCK+32], chr[2], word[32], *cryptedPassword;
    ssize_t readChr, length=SIZE_OF_BLOCK;
    int i, isCorrect=FALSE;
    chr[1]='\0';
    strcpy(word, "");

    while(length == SIZE_OF_BLOCK && isCorrectPassword == FALSE) {
        pthread_mutex_lock(&fileMutex);
        length = read(file, data, SIZE_OF_BLOCK);
        readBytes += length;
        data[length]='\0';
        if(length == SIZE_OF_BLOCK) {
            while (data[strlen(data) - 1] != '\n') {
                readChr = read(file, chr, 1);
                if(readChr == 0) break;
                strcat(data, chr);
                ++readBytes;
            }
        }
        printProgressPercent();
        pthread_mutex_unlock(&fileMutex);

        for (i = 0; i < strlen(data); ++i) {
            chr[0] = data[i];
            strcat(word, chr);
            if(chr[0]=='\n'){
                word[strlen(word)-1]='\0';
                comparePasswordAndHash(word, hash);
                strcpy(word, "");
            }
            if(isCorrectPassword) {
                pthread_exit(NULL);
            }
        }
    }
    if(strcmp(word, "") != 0){
        comparePasswordAndHash(word, hash);
    }
    pthread_exit(NULL);
}

int main(int argv,  char **argc){
    char *nameOfFile;
    int numberOfThreads, i;
    hash = argc[1];
    nameOfFile = argc[2];
    numberOfThreads = atoi(argc[3]);

    pthread_t threads[numberOfThreads];
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&fileMutex, NULL);
    pthread_mutex_init(&printMutex, NULL);

    struct stat fileStat;
    stat(nameOfFile, &fileStat);
    sizeOfFile = fileStat.st_size;
    file = open(nameOfFile, O_RDONLY);

    printProgressPercent();
    for(i=0; i<numberOfThreads; ++i){
        pthread_create(&threads[i], &threadAttr, threadFunction, NULL);
    }
    pthread_attr_destroy(&threadAttr);
    for(i=0; i<numberOfThreads; ++i){
        pthread_join(threads[i], NULL);
    }

    close(file);
    pthread_mutex_destroy(&fileMutex);
    pthread_mutex_destroy(&printMutex);

    printProgressPercent();
    if(isCorrectPassword){
        printf("\nThe password is %s\n", correctWord);
    }
    else{
        printf("\nThe password is not guessed\n");
    }
    return 0;
}
