#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include "values.h"

#define clear() printf("\033[H\033[J")
#define gotoxy(x, y) printf("\033[%d;%dH", x, y)

void print_board(char *board, char *position, char *info);
void quit(int s);

char *boardShm, *dataShm;
int id=-1;
sem_t *mutex;

char *positions[]={
        "up-left",
        "up-right",
        "down-right",
        "down-left"
};
char *movementInfo =
        "Now it is your turn.\nPlease chose a move:\n1: up-left 2: up 3: up-right 4: right 5: down-right 6: down 7: down-left 8: left\n";
char *waitInfo = "Wait for your turn.\n";
char *lostInfo = "You walk on mine. You have lost.\n";
char *endInfo = "The game has ended. You have lost.\n";
char *wrongMoveInfo = "This move is not possible. Choose another.\n";
char *winInfo = "You have won the game!\n";

int main(int argc, char **argv){
    int shmidBoard, shmidData, move, ret, isInGame=1;
    char positionInfo[64];
    sigset_t set;
    struct sigaction act;
    struct timespec sleepTime;

    sigemptyset(&set);
    act.sa_handler = &quit;
    act.sa_mask=set;
    act.sa_flags=0;
    sigaction(SIGINT, &act, NULL);

    sleepTime.tv_nsec = 5000000;
    sleepTime.tv_sec=0;

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

    mutex = sem_open(SEM_NAME,0,0666,0);

    char lastID= dataShm[LAST_ID];
    id=lastID;
    do{
        if(id==3)id=0;
        else ++id;
        if(dataShm[id*LENGTH_OF_DATA+IS_ACTIVE]==0) break;
    }while(id==lastID);
    if(dataShm[id*LENGTH_OF_DATA+IS_ACTIVE]==0){
        sprintf(positionInfo, "You are player number %d. Your begin position is %s.\n", id+1, positions[id]);
    }
    else{
        printf("You can not connect to server. Four people has connected.\n");
        quit(0);
    }
    sem_wait(mutex);
    dataShm[LAST_ID]=(char)id;
    dataShm[id*LENGTH_OF_DATA + IS_ACTIVE]=1;
    sem_post(mutex);
    while(isInGame){
        switch (dataShm[id*LENGTH_OF_DATA+STATE_OF_PLAYER]) {
            case WAIT:
                print_board(boardShm, positionInfo, waitInfo);
                break;
            case MOVE:
                print_board(boardShm, positionInfo, movementInfo);
                sem_wait(mutex);
                dataShm[id * LENGTH_OF_DATA + STATE_OF_PLAYER] = WAIT;
                sem_post(mutex);
                do {
                    ret = scanf("%d", &move);
                } while (ret < 1 || move < 1 || move > 8);
                sem_wait(mutex);
                dataShm[id * LENGTH_OF_DATA + MOVEMENT] = (char)move;
                sem_post(mutex);
                break;
            case WRONG_MOVE:
                print_board(boardShm, positionInfo, wrongMoveInfo);
                sem_wait(mutex);
                dataShm[id * LENGTH_OF_DATA + STATE_OF_PLAYER] = WAIT;
                sem_post(mutex);
                do {
                    ret = scanf("%d", &move);
                } while (ret < 1 || move < 1 || move > 8);
                sem_wait(mutex);
                dataShm[id * LENGTH_OF_DATA + MOVEMENT] = (char)move;
                sem_post(mutex);
                break;
            case WIN:
                print_board(boardShm, positionInfo, winInfo);
                isInGame=0;
                break;
            case MINE:
                print_board(boardShm, positionInfo, lostInfo);
                isInGame=0;
                break;
            case LOSE:
                print_board(boardShm, positionInfo, endInfo);
                isInGame=0;
                break;
        }
        nanosleep(&sleepTime, NULL);
    }
    quit(0);
}

void print_board(char *board, char *position, char *info) {
    int i,j;
    clear();
    gotoxy(0, 0);
    for(i=0; i<COLUMN_SIZE; ++i){
        for(j=0; j<ROW_SIZE; ++j){
            printf("%c", board[i*ROW_SIZE+j]);
        }
        printf("\n");
    }
    printf("%s\n%s\n", position, info);
    fflush(stdout);
}

void quit(int s) {
    sem_wait(mutex);
    if(id>=0 && id <= PLAYERS) dataShm[id*LENGTH_OF_DATA + IS_ACTIVE]=0;
    sem_post(mutex);
    shmdt(boardShm);
    shmdt(dataShm);
    sem_close(mutex);
    exit(0);
}
