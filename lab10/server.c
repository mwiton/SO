#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>
#include "values.h"

#define clear() printf("\033[H\033[J")
#define gotoxy(x, y) printf("\033[%d;%dH", x, y)

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
sem_t *mutex;

void quit(int s);
int getRandomField();
void boardInit(struct field board[][ROW_SIZE]);
void set_mines(struct field board[][ROW_SIZE]);
void calculateNumbers(struct field board[][ROW_SIZE]);
int getNumber(struct field board[][ROW_SIZE], int row, int column);



void saveIntoShm(struct field board[][ROW_SIZE], char* shm);

void dataShmInit();

void playersInit(struct player pPlayer[4]);

int makeMove(struct player *p, char movement, struct field board[11][21]);

int checkWin(struct player *p);

void endGame(int id);

int checkMine(struct player *pPlayer, struct field board[11][21]);

void deleteMine(int row, int column, struct field board[11][21]);

void showField(int row, int column, struct field board[11][21]);

void printPositions(struct player pPlayer[4], struct field pField[11][21]);

void playerReset(struct player *pPlayer, int id);

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

    mutex = sem_open(SEM_NAME, O_CREAT, 0666, 1);

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
            showField(players[id].row, players[id].column, board);
            saveIntoShm(board, boardShm);
            sem_wait(mutex);
            dataShm[id*LENGTH_OF_DATA+STATE_OF_PLAYER]=MOVE;
            sem_post(mutex);
            while(dataShm[id*LENGTH_OF_DATA + IS_ACTIVE]==1){
                if(dataShm[id*LENGTH_OF_DATA+MOVEMENT]!=0){
                    returnOfMove = makeMove(&players[id], dataShm[id*LENGTH_OF_DATA+MOVEMENT], board);
                    sem_wait(mutex);
                    dataShm[id*LENGTH_OF_DATA+MOVEMENT]=0;
                    sem_post(mutex);
                    if(returnOfMove==1) {
                        break;
                    }
                    else {
                        sem_wait(mutex);
                        dataShm[id * LENGTH_OF_DATA + STATE_OF_PLAYER] = WRONG_MOVE;
                        sem_post(mutex);
                    }
                }
            }
            if(dataShm[id*LENGTH_OF_DATA + IS_ACTIVE]==0) continue;
            printPositions(players, board);
            saveIntoShm(board, boardShm);
            if(checkWin(&players[id])){
                endGame(id);
                break;
            }
            if(checkMine(&players[id], board)){
                sem_wait(mutex);
                dataShm[id*LENGTH_OF_DATA+STATE_OF_PLAYER]=MINE;
                sem_post(mutex);
                deleteMine(players[id].row, players[id].column, board);
                sem_wait(mutex);
                dataShm[id*LENGTH_OF_DATA+IS_ACTIVE]=0;
                sem_post(mutex);
                playerReset(&players[id], id);
                saveIntoShm(board, boardShm);
            }
        }
        if(id==3) id=0;
        else ++id;
    }
    quit(0);
}

void playerReset(struct player *pPlayer, int id) {
    if(id==0) {
        pPlayer->column = 0;
        pPlayer->row = 0;
    }
    if(id==1){
        pPlayer->column=ROW_SIZE-1;
        pPlayer->row=0;
    }
    if(id==2) {
        pPlayer->column = ROW_SIZE - 1;
        pPlayer->row = COLUMN_SIZE - 1;
    }
    else {
        pPlayer->column = 0;
        pPlayer->row = COLUMN_SIZE - 1;
    }
}

void printPositions(struct player pPlayer[4], struct field pField[11][21]) {
    int i;
    clear();
    gotoxy(0, 0);
    for (i=0; i<PLAYERS; ++i){
        printf("id: %d row: %d vol: %d\n", i, pPlayer[i].row, pPlayer[i].column);
    }
}

void showField(int row, int column, struct field board[11][21]) {
    board[row][column].isVisible=1;
}

void deleteMine(int row, int column, struct field board[11][21]) {
    board[row][column].isMine=0;
    if(row!=0 && column!=0) --board[row-1][column-1].numberOfMines;
    if(row!=0) --board[row-1][column].numberOfMines;
    if(row!=0 && column!=ROW_SIZE-1) --board[row-1][column+1].numberOfMines;
    if(column!=ROW_SIZE-1) --board[row][column+1].numberOfMines;
    if(row!=COLUMN_SIZE-1 && column!=ROW_SIZE-1) --board[row+1][column+1].numberOfMines;
    if(row!=COLUMN_SIZE-1) --board[row+1][column].numberOfMines;
    if(row!=COLUMN_SIZE-1 && column!=0) --board[row+1][column-1].numberOfMines;
    if(column!=0) --board[row][column-1].numberOfMines;
}

int checkMine(struct player *pPlayer, struct field board[11][21]) {
    return board[pPlayer->row][pPlayer->column].isMine;
}

void endGame(int id) {
    int i;
    for(i=0; i<PLAYERS; ++i){
        if(i==id) dataShm[i*LENGTH_OF_DATA+STATE_OF_PLAYER]=WIN;
        else dataShm[i*LENGTH_OF_DATA+STATE_OF_PLAYER]=LOSE;
    }
}

int checkWin(struct player *p) {
    int middleRow = ROW_SIZE/2, middleColumn = COLUMN_SIZE/2;
    return p->row==middleColumn && p->column==middleRow;
}

int makeMove(struct player *p, char movement, struct field board[11][21]) {
    int row=p->row, column=p->column;
    switch(movement){
        case 1:
            --row;
            --column;
            break;
        case 2:
            --row;
            break;
        case 3:
            --row;
            ++column;
            break;
        case 4:
            ++column;
            break;
        case 5:
            ++row;
            ++column;
            break;
        case 6:
            ++row;
            break;
        case 7:
            ++row;
            --column;
            break;
        case 8:
            --column;
            break;
    }
    if(row<0 || row>=COLUMN_SIZE || column<0 || column>=ROW_SIZE) return 0;
    p->row=row;
    p->column=column;
    board[row][column].isVisible=1;
    return 1;
}

void playersInit(struct player players[PLAYERS]) {
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
    }
    dataShm[LAST_ID]=-1;
}

void quit(int s) {
    sem_close(mutex);
    sem_unlink(SEM_NAME);
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
            board[i][j].isVisible =0;
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
            board[i][j].numberOfMines = getNumber(board, i, j);
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