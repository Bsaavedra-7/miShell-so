#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "shell.h"
#include <unistd.h>
#include "separateCommands.h"


// Funcion para crear un proceso hijo (para que mishell.c no sea tan largo)
void execute(struct Command ** commands, Job * jobs, int comQuant) {
    for (int i = 0; i < comQuant; i++) {
        int j = 0;
        struct Command * comms = commands[i];
        while (1) {
            struct Command command = comms[j];
            if (strcmp(command.command, "cd") == 0) {
                if (strcmp(command.args[0], "") == 0) {
                    char * home = getenv("HOME");
                    if (home == NULL) {
                        printf("ERROR: No se encontro HOME\n");
                        continue;
                    }
                    if(chdir(home) == -1) {
                        perror("ERROR AL CAMBIAR DIRECTORIO");
                    }
                }
            }
            ++j;
            if (command.pipe == 0) {break;}
        }

    }

}