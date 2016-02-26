#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/resource.h>
#include <dlfcn.h>

rlim_t (*funcC)();
rlim_t (*funcD)();
rlim_t (*funcN)();
rlim_t (*funcS)();
rlim_t (*funcT)();
rlim_t (*funcU)();

void print_rlim_t(rlim_t x){
    if(x == RLIM_INFINITY){
        printf("unlimited\n");
    }
    else{
        printf("%lld\n", (long long) x);
    }
}

int main(int argc, char **argv){
    int ret;
    opterr = 0;
    void *handle = dlopen("./libulimit.so.0.1", RTLD_LAZY);
    if(!handle){
        dlerror();
        return 0;
    }
    funcC = (rlim_t)dlsym(handle, "funcC");
    funcD = (rlim_t)dlsym(handle, "funcD");
    funcN = (rlim_t)dlsym(handle, "funcN");
    funcS =(rlim_t) dlsym(handle, "funcS");
    funcT = (rlim_t)dlsym(handle, "funcT");
    funcU = (rlim_t)dlsym(handle, "funcU");
    while((ret = getopt(argc, argv, "acdnstu")) != -1){
        switch(ret){
            case 'a':
                printf("Maximum size of core file:\t");
                print_rlim_t((*funcC)());
                printf("Maximum size of segment data:\t");
                print_rlim_t((*funcD)());
                printf("Maximum number of open file descriptors:\t");
                print_rlim_t((*funcN)());
                printf("Maximum size of stack:\t");
                print_rlim_t((*funcS)());
                printf("Maximum amount of CPU time:\t");
                print_rlim_t((*funcT)());
                printf("Maximum number of processes:\t");
                print_rlim_t((*funcU)());
                break;
            case 'c':
                printf("Maximum size of core file:\t");
                print_rlim_t((*funcC)());
                break;
            case 'd':
                printf("Maximum size of segment data:\t");
                print_rlim_t((*funcD)());
                break;
            case 'n':
                printf("Maximum number of open file descriptors:\t");
                print_rlim_t((*funcN)());
                break;
            case 's':
                printf("Maximum size of stack:\t");
                print_rlim_t((*funcS)());
                break;
            case 't':
                printf("Maximum amount of CPU time:\t");
                print_rlim_t((*funcT)());
                break;
            case 'u':
                printf("Maximum size of core file:\t");
                print_rlim_t((*funcU)());
                break;
            case '?':
                printf("[brak obslugi parametru -%c]\n", optopt);
        }
    }
    dlclose(handle);
    return 0;
}