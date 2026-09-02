#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "shell.h"
#include <unistd.h>


//  gcc mishell.c -o mishell
//  ./mishell


int main(void) {
    
    //ruta actual con getcwd
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
        return 1;
    }



    while (1) {
        char line[MAX_LINE];
        printf("miShell> ");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break; 
        }
        
        line[strcspn(line, "\n")] = 0;

        printf("You entered: %s\n", line);
    }



    return 0;
}