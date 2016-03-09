#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv){
    int i;
    int n = atoi(argv[1]);
    pid_t child1 = (pid_t )0, child2 = (pid_t )0;
    char **tab, str_n[10], str_pid[10];
    if(n>0){
        child1 = fork();
        if(child1 > 0) {
            child2 = fork();
        }
        if((int)child2 == 0){
            tab = malloc((argc+2) * sizeof(char *));
            sprintf(str_n, "%d", n - 1);
            tab[0]=argv[0];
            tab[1] = str_n;
            for (i = 2; i < argc; ++i) {
                tab[i] = argv[i];
            }
            sprintf(str_pid,"%d",(int)getppid());
            tab[i] = str_pid;
            tab[i+1] = NULL;
            execv(argv[0], tab);
        }
    }
    if(child1 > 0 && child2 > 0){
        waitpid(child1, NULL, 0);
        waitpid(child2, NULL, 0);
    }
    for(i=2; i<argc; ++i){
        printf("%s ", argv[i]);
    }
    printf("%d\n", (int)getpid());
    return 0;
}

