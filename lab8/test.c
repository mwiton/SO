#include <stdio.h>
#include <unistd.h>
#include <crypt.h>
#include <string.h>

int main(int argv, char** argc)
{
    char salt[19], password[32];
    char* encryptedPassword;
    int i;

    strcpy(salt, "$6$");
    strcpy(password, argc[1]);
    strcat(salt, argc[2]);

    /* Turn it into printable characters from ‘seedchars’. */
    /* Read in the user’s password and encrypt it. */
    encryptedPassword = crypt(password, salt);

    /* Print the results. */
    puts(encryptedPassword);
    return 0;
}

