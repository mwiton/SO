#include <stdio.h>
#include <unistd.h>
#include <crypt.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define SIZE_OF_BLOCK 128
#define FALSE 0
#define TRUE 1

int main(int argv,  char **argc){
    char *hash, *nameOfFile, data[5050], chr[2], word[20], *cryptedPassword;
    int threads, file, i, isCorrect=FALSE;
    ssize_t length=SIZE_OF_BLOCK, readChr;
    hash = argc[1];
    nameOfFile = argc[2];
    threads = atoi(argc[3]);
    chr[1]='\0';
    strcpy(word, "");
    file = open(nameOfFile, O_RDONLY);
    while(length == SIZE_OF_BLOCK && isCorrect == FALSE) {
        length = read(file, data, SIZE_OF_BLOCK);
        data[length]='\0';
        if(length == SIZE_OF_BLOCK) {
            while (data[strlen(data) - 1] != '\n') {
                readChr = read(file, chr, 1);
                if(readChr == 0) break;
                strcat(data, chr);
            }
        }
        for (i = 0; i < strlen(data); ++i) {
            chr[0] = data[i];
            strcat(word, chr);
            if(chr[0]=='\n'){
                printf("%s", word);
                word[strlen(word)-1]='\0';
                cryptedPassword = crypt(word, hash);
                if (strcmp(cryptedPassword, hash) == 0){
                    isCorrect = TRUE;
                    break;
                }
                strcpy(word, "");
            }
        }
    }
    close(file);
    if(!isCorrect && strcmp(word, "")!=0){
        cryptedPassword = crypt(word, hash);
        if (strcmp(cryptedPassword, hash) == 0){
            isCorrect = TRUE;
        }
    }
    if(isCorrect){
        printf("The password is %s\n", word);
    }
    else{
        printf("The password is not guessed\n");
    }
    return 0;
}
