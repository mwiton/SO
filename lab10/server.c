#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <stdbool.h>
#include "values.h"

struct field{
    int numberOfMines;
    int isMine;
    int isVisible;
};

struct player{
    int row;
    int column;
};

char *boardShm, *dataShm;
int boardShmId, dataShmId;

void quit(int s);
int getRandomField();
void boardInit(struct field board[][ROW_SIZE]);
void set_mines(struct field board[][ROW_SIZE]);
void calculateNumbers(struct field board[][ROW_SIZE]);
int getNumber(struct field board[][ROW_SIZE], int row, int column);



void saveIntoShm(struct field board[][ROW_SIZE], char* shm);

void dataShmInit();

void playersInit(struct player pPlayer[4]);

int makeMove(struct player *p, char movement);

int checkWin(struct player *p);

void endGame(int id);

void checkMine(struct player *pPlayer, int id, struct field board[11][21]);

int main(int argc, char **argv){
    int id=0, returnOfMove;
    struct field board[COLUMN_SIZE][ROW_SIZE];
    sigset_t set;
    struct sigaction act;
    struct player players[PLAYERS];

    sigemptyset(&set);
    act.sa_handler = &quit;
    act.sa_mask=set;
    act.sa_flags=0;
    sigaction(SIGINT, &act, NULL);


    if ((boardShmId = shmget(KEY_BOARD, SIZE, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    if ((boardShm = shmat(boardShmId, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    if ((dataShmId = shmget(KEY_DATA, LAST_ID+1, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    if ((dataShm = shmat(dataShmId, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    dataShmInit();
    boardInit(board);
    playersInit(players);
    set_mines(board);
    calculateNumbers(board);
    saveIntoShm(board, boardShm);
    while(1) {
        if(dataShm[id*LENGTH_OF_DATA + IS_ACTIVE]==1){
            dataShm[id*LENGTH_OF_DATA+STATE_OF_PLAYER]=1;
            while(dataShm[id*LENGTH_OF_DATA + IS_ACTIVE]==1){
                if(dataShm[id*LENGTH_OF_DATA+MOVEMENT]!=0){
                    //dataShm[id*LENGTH_OF_DATA+STATE_OF_PLAYER]=0;
                    returnOfMove = makeMove(&players[id], dataShm[id*LENGTH_OF_DATA+MOVEMENT]);
                    if(returnOfMove==1) {
                        dataShm[id*LENGTH_OF_DATA+CORRECT_MOVEMENT]=1;
                        break;
                    }
                    dataShm[id*LENGTH_OF_DATA+CORRECT_MOVEMENT]=-1;
                }
            }
            if(dataShm[id*LENGTH_OF_DATA + IS_ACTIVE]==0) continue;
            if(checkWin(&players[id])){
                endGame(id);
                break;
            }
            checkMine(&players[id], id, board);
        }
        if(id==3) id=0;
        else ++id;
    }
    quit(0);
}

void checkMine(struct player *pPlayer, int id, struct field board[11][21]) {

}

void endGame(int id) {

}

int checkWin(struct player *p) {
    return 0;
}

int makeMove(struct player *p, char movement) {

}

void playersInit(struct player players[PLAYERS]) {
    int i;
    players[0].column=0;
    players[0].row=0;
    players[1].column=ROW_SIZE-1;
    players[1].row=0;
    players[2].column=ROW_SIZE-1;
    players[2].row=COLUMN_SIZE-1;
    players[3].column=0;
    players[3].row=COLUMN_SIZE-1;
}

void dataShmInit() {
    int i;
    for(i=0; i<PLAYERS; ++i){
        dataShm[i*LENGTH_OF_DATA + IS_ACTIVE]=0;
        dataShm[i*LENGTH_OF_DATA + STATE_OF_PLAYER]=0;
        dataShm[i*LENGTH_OF_DATA + MOVEMENT]=0;
        dataShm[i*LENGTH_OF_DATA + CORRECT_MOVEMENT]=0;
    }
    dataShm[LAST_ID]=-1;
}

void quit(int s) {
    shmdt(boardShm);
    shmctl(boardShmId, IPC_RMID, NULL);
    shmdt(dataShm);
    shmctl(dataShmId, IPC_RMID, NULL);
    exit(0);
}

int getRandomField(){
    int field;
    do{
        field = rand()%SIZE;
    }while(field==0 || field==ROW_SIZE-1 || field==(COLUMN_SIZE-1)*ROW_SIZE || field==SIZE-1);
    return field;
}

void boardInit(struct field board[][ROW_SIZE]){
    int i, j;
    for(i=0; i<COLUMN_SIZE; ++i){
        for(j=0; j<ROW_SIZE; ++j){
            board[i][j].isMine=0;
            board[i][j].isVisible =1;
            board[i][j].numberOfMines=0;
        }
    }
}

void set_mines(struct field board[][ROW_SIZE]){
    int i, field, row, column;
    for(i=0; i<MINES; ++i){
        do{
            field = getRandomField();
            row=field/ROW_SIZE;
            column=field%ROW_SIZE;
        }while (board[row][column].isMine);
        board[row][column].isMine=1;
    }
}

void calculateNumbers(struct field board[][ROW_SIZE]) {
    int i,j;
    for(i=0; i<COLUMN_SIZE; ++i){
        for(j=0; j<ROW_SIZE; ++j){
            if(!board[i][j].isMine){
                board[i][j].numberOfMines = getNumber(board, i, j);
            }
        }
    }
}



int getNumber(struct field board[][ROW_SIZE], int row, int column) {
    int numberOfMines=0;
    if(row!=0 && column!=0) numberOfMines += board[row-1][column-1].isMine;
    if(row!=0) numberOfMines += board[row-1][column].isMine;
    if(row!=0 && column!=ROW_SIZE-1) numberOfMines += board[row-1][column+1].isMine;
    if(column!=ROW_SIZE-1) numberOfMines += board[row][column+1].isMine;
    if(row!=COLUMN_SIZE-1 && column!=ROW_SIZE-1) numberOfMines += board[row+1][column+1].isMine;
    if(row!=COLUMN_SIZE-1) numberOfMines += board[row+1][column].isMine;
    if(row!=COLUMN_SIZE-1 && column!=0) numberOfMines += board[row+1][column-1].isMine;
    if(column!=0) numberOfMines += board[row][column-1].isMine;
    return numberOfMines;
}

void saveIntoShm(struct field (*board)[21], char *shm) {
    int i,j;
    char number[2];
    for(i=0; i<COLUMN_SIZE; ++i){
        for(j=0; j<ROW_SIZE; ++j){
            if(!board[i][j].isVisible){
                shm[i*ROW_SIZE+j]='#';
            }
            else{
                if(board[i][j].isMine){
                    shm[i*ROW_SIZE+j]='*';
                }
                else{
                    sprintf(number, "%d", board[i][j].numberOfMines);
                    shm[i*ROW_SIZE+j]=number[0];
                }
            }
        }
    }
}