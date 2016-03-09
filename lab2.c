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

void print_rlim_t(rlim_t (*fun)(), char opt){
    if(fun){
	    rlim_t x=(*fun)();
	    if(x == RLIM_INFINITY){
		printf("unlimited\n");
	    }
	    else{
		printf("%d\n", (int) x);
	    }
    }
    else printf("[brak obslugi parametru -%c]\n", opt);
}

int main(int argc, char **argv){
    int ret;
    opterr = 0;
    void *handle = dlopen("./libulimit.so.0.1", RTLD_LAZY);
    if(!handle){
        dlerror();
        return 0;
    }
    funcC = dlsym(handle, "funcC");
    funcD = dlsym(handle, "funcD");
    funcN = dlsym(handle, "funcN");
    funcS = dlsym(handle, "funcS");
    funcT = dlsym(handle, "funcT");
    funcU = dlsym(handle, "funcU");
    while((ret = getopt(argc, argv, "acdnstu")) != -1){
        switch(ret){
            case 'a':
                printf("Maximum size of core file:\t");
                print_rlim_t(funcC, 'c');
                printf("Maximum size of segment data:\t");
                print_rlim_t(funcD, 'd');
                printf("Maximum number of open file descriptors:\t");
                print_rlim_t(funcN, 'n');
                printf("Maximum size of stack:\t");
                print_rlim_t(funcS, 's');
                printf("Maximum amount of CPU time:\t");
                print_rlim_t(funcT, 't');
                printf("Maximum number of processes:\t");
                print_rlim_t(funcU, 'u');
                break;
            case 'c':
                printf("Maximum size of core file:\t");
                print_rlim_t(funcC, 'c');
                break;
            case 'd':
                printf("Maximum size of segment data:\t");
                print_rlim_t(funcD, 'd');
                break;
            case 'n':
                printf("Maximum number of open file descriptors:\t");
                print_rlim_t(funcN, 'n');
                break;
            case 's':
                printf("Maximum size of stack:\t");
                print_rlim_t(funcS, 's');
                break;
            case 't':
                printf("Maximum amount of CPU time:\t");
                print_rlim_t(funcT, 't');
                break;
            case 'u':
                printf("Maximum number of processes:\t");
                print_rlim_t(funcU, 'u');
                break;
            case '?':
                printf("[brak obslugi parametru -%c]\n", optopt);
        }
    }
    dlclose(handle);
    return 0;
}
