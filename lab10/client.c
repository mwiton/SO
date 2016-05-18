#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include "values.h"


void print_board(char *board, char *position);
void quit(int s);

char *boardShm, *dataShm;
int id=-1;

char *positions[]={
        "up-left",
        "up-right",
        "down-right",
        "down-left"
};
char *movementInfo = {
        "Now it is your turn.\nPlease chose a move:\n1: up-left 2: up 3: up-right 4: right 5: down-right 6: down 7: down-left 8: left\n"};
char *waitInfo = {"Wait for your turn.\n"};
char *lostInfo = {"You walk on mine. You have lost.\n"};
char *endInfo = "The game has ended\n";

int main(int argc, char **argv){
    int shmidBoard, shmidData;
    char positionInfo[64];
    sigset_t set;
    struct sigaction act;

    sigemptyset(&set);
    act.sa_handler = &quit;
    act.sa_mask=set;
    act.sa_flags=0;
    sigaction(SIGINT, &act, NULL);

    if ((shmidBoard = shmget(KEY_BOARD, SIZE, 0666)) < 0) {
        printf("The server is not running.\n");
        exit(1);
    }

    if ((boardShm = shmat(shmidBoard, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    if ((shmidData = shmget(KEY_DATA, LAST_ID+1, 0666)) < 0) {
        printf("The server is not running.\n");
        exit(1);
    }

    if ((dataShm = shmat(shmidData, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }


    id = (int)dataShm[LAST_ID]+1;
    if(id<PLAYERS){
        sprintf(positionInfo, "You are player number %d. Your begin position is %s.\n", id+1, positions[id]);
    }
    else{
        printf("You can not connect to server. Four people has connected.\n");
        quit(0);
    }
    dataShm[LAST_ID]=(char)id;
    dataShm[id*LENGTH_OF_DATA + IS_ACTIVE]=1;

    print_board(boardShm, positionInfo);
    while(1){}
    return 0;
}

void print_board(char *board, char *position) {
    int i,j;
    for(i=0; i<COLUMN_SIZE; ++i){
        for(j=0; j<ROW_SIZE; ++j){
            printf("%c", board[i*ROW_SIZE+j]);
        }
        printf("\n");
    }
    printf("%s\n", position);
}

void quit(int s) {
    if(id>=0 && id <= PLAYERS) dataShm[id*LENGTH_OF_DATA + IS_ACTIVE]=0;
    shmdt(boardShm);
    shmdt(dataShm);
    exit(0);
}
