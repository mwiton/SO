#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <string.h>

int main(int argc, char **argv){
	int ret, i, gvalue = 0, svalue = 0;
	char *sstr;
	gid_t group;
	struct group *structGroup;
	struct passwd *user;
	gid_t *groups;
	int *ngroups;
    ngroups = malloc(sizeof(int));
	*ngroups = 100;
	while((ret = getopt(argc, argv, "gs:")) != -1){
		switch(ret){
			case 'g':
                gvalue = 1;
                break;
			case 's':
                svalue = 1;
                sstr = optarg;
                break;
            default:
                printf("Brak takiej opcji\n");
                return -1;
		}
	}
	while((user = getpwent()) != '\0'){
        if(svalue){
            if(strcmp(sstr, user->pw_shell) != 0){
                continue;
            }
        }
		printf("%s %s ", user->pw_name, user->pw_shell);
		if(gvalue){
			group = user->pw_gid;
            groups = malloc(*ngroups * sizeof(gid_t));
			getgrouplist(user->pw_name, group, groups, ngroups);
			for(i=0; i<*ngroups; ++i) {
				structGroup = getgrgid(groups[i]);
                printf("%s ", structGroup->gr_name);
			}
            free(groups);
		}
	printf("\n");	
	}
    free(ngroups);
	return 0;
}
