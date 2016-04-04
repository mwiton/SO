#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>


int main(int argc, char **argv){
    DIR* proc;
    char fileName[512], nameProcess[512], nameProgram[8];
    int file, length, killall=0;
    strcpy(nameProgram, &argv[0][strlen(argv[0])-7]);
    if(strcmp(nameProgram, "killall")==0){
        killall=1;
    }
    proc = opendir("/proc");
    struct dirent *entry;
    while ((entry = readdir(proc)) != NULL){
        if('1' <= entry->d_name[0] && entry->d_name[0] <= '9'){
            stpcpy(fileName, "/proc/");
            strcat(fileName, entry->d_name);
            strcat(fileName, "/comm");
            file = open(fileName, O_RDONLY);
            length = read(file, nameProcess, 512);
            close(file);
            nameProcess[length-1]='\0';
            if(strcmp(argv[1], nameProcess) == 0){
                printf("%s\n", entry->d_name);
                if(killall){
                    kill((pid_t)atoi(entry->d_name), SIGINT);
                }
            }
        }
    }
    closedir(proc);
    return 0;
}

