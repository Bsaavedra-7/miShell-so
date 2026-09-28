#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
//#include <sys/wait.h>
#include "shell.h"
#include "child.h"
#include <signal.h>
#include <unistd.h>
#include "validar.h"

int sepCmds(char **args, Command *** commands, int argq) { // Diferencia comandos de argumentos
    const char* builtins[] = {"cd", "exit", "jobs", "pmon"};
    int builtinsq = 4;
    Command commandUnfinished; // para despues hacer malloc y añadirlo al array
    int commandsq = 0, endOfCommand = 0, isPipe = 0, skipLoop = 0, nextCommandisBackground = 0;
    int argsq = 0; // variable para contar argumentos de cada comando
    char **commandArgs = (char *) malloc(sizeof(char *) * MAX_ARG_LEN * MAX_ARGS);
    // 
    for (int i = 0; i < argq; i++) {
        if (skipLoop == 1) {skipLoop = 0; continue;}
        if (!es_comando(args[i])) { // Si no es un comando
            if (strcmp(args[i], "|") == 0) {
                ++isPipe; 
                //TODO agregar comando a array de pipe
                continue;
            } 
            if (strcmp(args[i], ">") == 0) {
                commandUnfinished->input = args[i + 1]; 
                commandUnfinished->inputAppend = 0;
                skipLoop = 1;
                continue;
            }
            if (strcmp(args[i], ">>") == 0) {
                commandUnfinished->input = args[i + 1];
                commandUnfinished->inputAppend = 1;
                skipLoop = 1;
                continue;
            }
            if (strcmp(args[i], "<") == 0) {
                commandUnfinished->output = args[i + 1];
                skipLoop = 1;
                continue;
            }

            commandArgs[argsq] = args[i];
            ++ argsq;
            continue;

        } else { // Si es comando
            argsq = 0;
            commandUnfinished->args = commandArgs;
            commandArgs = (char *) malloc(sizeof(char *) * MAX_ARG_LEN * MAX_ARGS);
        }

        for (int j = 0; j < argq; j++) { // Ciclo para comparar con builtins
            if (strcmp(args[i], builtins[j]) == 0) {
                commandUnfinished->command = args[i];   
                ++commandsq;
                continue;
            }

        }

        if (strcmp(args[i], "&") == 0 | strcmp(args[i], "&&") == 0) {
            isPipe = 0;
            nextCommandisBackground = 1;

        }

    }

}