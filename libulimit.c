#include "libulimit.h"

rlim_t funcC(){
    struct rlimit *rlim;
    rlim = malloc(sizeof(struct rlimit));
    getrlimit(RLIMIT_CORE, rlim);
    rlim_t ret = rlim->rlim_cur;
    free(rlim);
    return ret;
}

rlim_t funcD(){
    struct rlimit *rlim;
    rlim = malloc(sizeof(struct rlimit));
    getrlimit(RLIMIT_DATA, rlim);
    rlim_t ret = rlim->rlim_cur;
    free(rlim);
    return ret;}

rlim_t funcN(){
    struct rlimit *rlim;
    rlim = malloc(sizeof(struct rlimit));
    getrlimit(RLIMIT_NOFILE, rlim);
    rlim_t ret = rlim->rlim_cur;
    free(rlim);
    return ret;}

rlim_t funcS(){
    struct rlimit *rlim;
    rlim = malloc(sizeof(struct rlimit));
    getrlimit(RLIMIT_STACK, rlim);
    rlim_t ret = rlim->rlim_cur;
    free(rlim);
    return ret;}

rlim_t funcT(){
    struct rlimit *rlim;
    rlim = malloc(sizeof(struct rlimit));
    getrlimit(RLIMIT_CPU, rlim);
    rlim_t ret = rlim->rlim_cur;
    free(rlim);
    return ret;}

rlim_t funcU(){
    struct rlimit *rlim;
    rlim = malloc(sizeof(struct rlimit));
    getrlimit(RLIMIT_NPROC, rlim);
    rlim_t ret = rlim->rlim_cur;
    free(rlim);
    return ret;}