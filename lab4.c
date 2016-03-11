#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

pid_t child1 = (pid_t )0, child2 = (pid_t )0;
char **parents;
int numParents;

void sigintHandler(int s){
    int i;
    if(child1 != 0) kill(child1, SIGINT);
    if(child2 != 0) kill(child2, SIGINT);
    if(child1 > 0 && child2 > 0){
        waitpid(child1, NULL, 0);
        waitpid(child2, NULL, 0);
    }
    for(i=2; i<numParents; ++i){
        printf("%s ", parents[i]);
    }
    printf("%d\n", (int)getpid());
    exit(0);
}

int main(int argc, char **argv){
    int i, n = atoi(argv[1]);
    char **tab, str_n[10], str_pid[10];
    sigset_t set, blockedSet;
    struct sigaction act;
    parents = argv;
    numParents = argc;

    sigemptyset(&set);
    act.sa_handler = &sigintHandler;
    act.sa_mask=set;
    act.sa_flags=0;
    sigaction(SIGINT, &act, NULL);

    sigemptyset(&blockedSet);
    sigaddset(&blockedSet, SIGTSTP);
    sigprocmask(SIG_BLOCK, &blockedSet, NULL);

    if(n>0){
        child1 = fork();
        if(child1 > 0) {
            child2 = fork();
        }
        if((int)child2 == 0){
            setpgid(0, 0);
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

    while (1){ }
    return 0;
}



