#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "shell.h"
#include <unistd.h>


//  gcc mishell.c -o mishell
//  ./mishell

int tokenizar (char *line, char **args){

    line[strcspn(line, "\n")] = 0; // Eliminar el salto de linea

    //aca tokenizamos el comando, determinando espacios, tabs y salto de linea
    char *token = strtok(line, "\t\r\n "); 

    int i = 0;
    //el ultimo arg tiene que ser null
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i] = token;
        token = strtok(NULL, "\t\r\n ");
        i++;
        printf("Token %d: %s\n", i, args[i-1]); // Imprimir cada token
    }
    args[i] = NULL;
    return i;
}



int main(void) {
    
    //ruta actual con getcwd
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
        return 1;
    }


    char *args[MAX_ARGS];
    
    while (1) {

        char line[MAX_LINE];
        printf("miShell> ");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break; 
        }
        
        line[strcspn(line, "\n")] = 0;

        int argc = tokenizar(line, args);
        printf("Number of arguments: %d\n", argc);

        printf("You entered: %s\n", line);
    }



    return 0;
}