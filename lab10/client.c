#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>


#define KEY 5678
#define SIZE 231
#define COLUMN_SIZE 11
#define ROW_SIZE 21

void print_board(char* board);
void sigintHandler(int s);

char *boardShm;

int main(int argc, char **argv){
    int shmid;
    sigset_t set;
    struct sigaction act;

    sigemptyset(&set);
    act.sa_handler = &sigintHandler;
    act.sa_mask=set;
    act.sa_flags=0;
    sigaction(SIGINT, &act, NULL);

    if ((shmid = shmget(KEY, SIZE, 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    if ((boardShm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }
    print_board(boardShm);
    while(1){}
    return 0;
}

void print_board(char* board){
    int i,j;
    for(i=0; i<COLUMN_SIZE; ++i){
        for(j=0; j<ROW_SIZE; ++j){
            printf("%c", board[i*ROW_SIZE+j]);
        }
        printf("\n");
    }
}

void sigintHandler(int s) {
    shmdt(boardShm);
    exit(0);
}
