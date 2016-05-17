#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <crypt.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>

#define SIZE_OF_BLOCK 4096
#define TESTED_PASSWORDS 1000

#define clear() printf("\033[H\033[J")
#define gotoxy(x, y) printf("\033[%d;%dH", x, y)

int file , isCorrectPassword = 0, sizeOfFile, readBytes=0, testTime =0, testedPasswords=0;
char *hash, correctWord[32];
pthread_mutex_t fileMutex, bytesMutex, passwordMutex;


void printProgressPercent(){
    float percent;
    percent = (float)readBytes/sizeOfFile*100;
    clear();
    gotoxy(0, 0);
    printf("Percent of read bytes: %f%%", percent);
    fflush(stdout);
}

void comparePasswordAndHash(char *password, char *hash, struct crypt_data cryptData){
    char* cryptedPassword, *tmp;
    tmp = cryptedPassword;
    cryptedPassword = crypt_r(password, hash, &cryptData);
    if (strcmp(cryptedPassword, hash) == 0){
        isCorrectPassword = 1;
        strcpy(correctWord, password);
    }
}

void *threadFunction(void *arg){
    char data[SIZE_OF_BLOCK+32], chr[2], *word;
    ssize_t readChr, length=SIZE_OF_BLOCK, lengthOfData;
    int i, beginOfWord;
    struct crypt_data cryptData;
    chr[1]='\0';
    cryptData.initialized=0;

    while(length == SIZE_OF_BLOCK && isCorrectPassword == 0) {
        pthread_mutex_lock(&fileMutex);
        length = read(file, data, SIZE_OF_BLOCK);
        data[length]='\0';
        if(length == SIZE_OF_BLOCK) {
            while (data[strlen(data) - 1] != '\n') {
                readChr = read(file, chr, 1);
                if(readChr == 0) break;
                strcat(data, chr);
            }
        }
        pthread_mutex_unlock(&fileMutex);
        beginOfWord = 0;

        lengthOfData = strlen(data);
        word = strtok(data, "\n\r");
        while(word != NULL) {
            comparePasswordAndHash(word, hash, cryptData);
            if(isCorrectPassword && !testTime) {
                pthread_mutex_lock(&bytesMutex);
                readBytes += lengthOfData;
                printProgressPercent();
                pthread_mutex_unlock(&bytesMutex);
                pthread_exit(NULL);
            }
            word = strtok(NULL, "\n\r");
            pthread_mutex_lock(&passwordMutex);
            ++testedPasswords;
            pthread_mutex_unlock(&passwordMutex);
            if(testTime && testedPasswords >= TESTED_PASSWORDS) pthread_exit(NULL);
        }
        pthread_mutex_lock(&bytesMutex);
        readBytes += lengthOfData;
        printProgressPercent();
        pthread_mutex_unlock(&bytesMutex);
    }
    pthread_exit(NULL);
}

int main(int argv,  char **argc){
    char *nameOfFile;
    int numberOfThreads, i, numberOfProcessors=(int)sysconf(_SC_NPROCESSORS_CONF);
    double *times;
    struct timeval  start, end;
    struct timespec ts;

    hash = argc[1];
    nameOfFile = argc[2];
    if(argv == 3){
        numberOfThreads=numberOfProcessors;
        testTime =1;
        times = malloc(numberOfProcessors* sizeof(double));
    }
    else numberOfThreads = atoi(argc[3]);
    if(numberOfThreads > numberOfProcessors){
        printf("Number of threads is bigger than number of processors\n");
        return -1;
    }

    pthread_t *threads=(pthread_t*)malloc(numberOfThreads* sizeof(pthread_t));
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&fileMutex, NULL);
    pthread_mutex_init(&bytesMutex, NULL);
    pthread_mutex_init(&passwordMutex, NULL);

    struct stat fileStat;
    stat(nameOfFile, &fileStat);
    sizeOfFile = fileStat.st_size;
    file = open(nameOfFile, O_RDONLY);
    if(testTime)numberOfThreads=1;
    do {
        testedPasswords=0;
        lseek(file, 0, 0);
        readBytes=0;
        gettimeofday(&start, NULL);
        for (i = 0; i < numberOfThreads; ++i) {
            pthread_create(threads+i, &threadAttr, threadFunction, NULL);
        }
        for (i = 0; i < numberOfThreads; ++i) {
            pthread_join(threads[i], NULL);
        }
        gettimeofday(&end, NULL);
        if(testTime){
            times[numberOfThreads-1]=(double) (end.tv_usec - start.tv_usec) / 1000000 +
                                     (double) (end.tv_sec - start.tv_sec);
            ++numberOfThreads;
        }
    } while(testTime && numberOfThreads <= numberOfProcessors);

    close(file);
    pthread_attr_destroy(&threadAttr);
    pthread_mutex_destroy(&fileMutex);
    pthread_mutex_destroy(&bytesMutex);
    pthread_mutex_destroy(&passwordMutex);

    printProgressPercent();
    if(testTime){
        printf("\n");
        for(i=0; i<numberOfProcessors; ++i){
            printf("Number of threads: %d, time: %f s\n", i+1, times[i]);
        }
        free(times);
    }
    else {
        if (isCorrectPassword) {
            printf("\nThe password is %s\n", correctWord);
        }
        else {
            printf("\nThe password is not guessed\n");
        }
    }
    free(threads);
    return 0;
}