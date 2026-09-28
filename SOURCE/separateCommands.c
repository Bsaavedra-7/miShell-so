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

void copy_array(char **from, char **to, int n) {
    for (int i = 0; i < n; i++) {
        from[i] = to[i];
    }
}

int sepCmds(char **args, Command *** commands, int argq) { // Diferencia comandos de argumentos
    const char* builtins[] = {"cd", "exit", "jobs", "pmon"};
    int builtinsq = 4;
    commands[0] = (Command *)malloc(sizeof(Command*));
    Command commandUnfinished; // para despues hacer malloc y añadirlo al array
    commandUnfinished->jobRunType = FOREGROUND;
    int commandsq = 0, endOfCommand = 0, isPipe = 0, skipLoop = 0, nextCommandisBackground = 0;
    int argsq = 0; // variable para contar argumentos de cada comando
    char **commandArgs = (char *) malloc(sizeof(char *) * MAX_ARG_LEN * MAX_ARGS);
    // inicializacion de instancia 
    commandUnfinished->args = commandArgs;
    commandUnfinished->command = "";
    commandUnfinished->input = "";
    commandUnfinished->inputAppend = 0;
    commandUnfinished->jobRunType = FOREGROUND;
    commandUnfinished->output = "";
    // 
    for (int i = 0; i < argq; i++) {
        if (skipLoop == 1) {skipLoop = 0; continue;}
        if (!es_comando(args[i])) { // Si no es un comando
            if (strcmp(args[i], "|") == 0) {
                argsq = 0;
                Command commandFinished = (Command) malloc(sizeof(Command));
                char ** cargs = malloc(sizeof(commandUnfinished->args));
                copy_array(cargs, commandUnfinished->args, argsq);
                commandFinished->args = cargs;
                char * ccmd = malloc(sizeof(commandUnfinished->command));
                strcpy(ccmd, commandUnfinished->command);
                commandFinished->command = ccmd;
                char * cinput = malloc(sizeof(commandUnfinished->input));
                strcpy(cinput, commandUnfinished->input);
                commandFinished->input = cinput;
                char * coutput = malloc(sizeof(commandUnfinished->output));
                strcpy(coutput, commandUnfinished->output);
                commandFinished->output = coutput;

                if (commandUnfinished->input == 0) {
                    commandFinished->input = 0;
                } else {commandFinished->input = 1;}
                if (commandUnfinished->jobRunType == BACKGROUND) {
                    commandFinished->jobRunType = BACKGROUND;
                } else {commandFinished->jobRunType = FOREGROUND;}
                
                commands[commandsq - 1] =  realloc(commands[commandsq -1], sizeof(commands[commandsq - 1]) + sizeof(commandFinished));
                commands[commandsq][isPipe] = commandFinished; 
                ++isPipe; 

                commandUnfinished->command = "";
                commandUnfinished->input = "";
                commandUnfinished->inputAppend = 0;
                commandUnfinished->jobRunType = FOREGROUND;
                commandUnfinished->output = "";
                commandArgs = (char *) malloc(sizeof(char *) * MAX_ARG_LEN * MAX_ARGS);
                continue;
            } 
            if (strcmp(args[i], ">") == 0) {
                commandUnfinished->input = args[i + 1]; 
                commandUnfinished->inputAppend = 0;
                skipLoop = 1;
                argsq = 0;
                continue;
            }
            if (strcmp(args[i], ">>") == 0) {
                commandUnfinished->input = args[i + 1];
                commandUnfinished->inputAppend = 1;
                skipLoop = 1;
                argsq = 0;
                continue;
            }
            if (strcmp(args[i], "<") == 0) {
                commandUnfinished->output = args[i + 1];
                skipLoop = 1;
                argsq = 0;
                continue;
            }
            if (strcmp(args[i], "&") == 0) {
                commandUnfinished->jobRunType = BACKGROUND;
                continue;

            }
            if (strcmp(args[i], "&&") == 0) {
                argsq = 0;
                Command commandFinished = (Command) malloc(sizeof(Command));
                char ** cargs = malloc(sizeof(commandUnfinished->args));
                copy_array(cargs, commandUnfinished->args, argsq);
                commandFinished->args = cargs;
                char * ccmd = malloc(sizeof(commandUnfinished->command));
                strcpy(ccmd, commandUnfinished->command);
                commandFinished->command = ccmd;
                char * cinput = malloc(sizeof(commandUnfinished->input));
                strcpy(cinput, commandUnfinished->input);
                commandFinished->input = cinput;
                char * coutput = malloc(sizeof(commandUnfinished->output));
                strcpy(coutput, commandUnfinished->output);
                commandFinished->output = coutput;

                if (commandUnfinished->input == 0) {
                    commandFinished->input = 0;
                } else {commandFinished->input = 1;}
                if (commandUnfinished->jobRunType == BACKGROUND) {
                    commandFinished->jobRunType = BACKGROUND;
                } else {commandFinished->jobRunType = FOREGROUND;}
                
                commands = realloc(commands, sizeof(commands) + sizeof(commandFinished));
                commands[commandsq][isPipe] = commandFinished; 

                commandUnfinished->command = "";
                commandUnfinished->input = "";
                commandUnfinished->inputAppend = 0;
                commandUnfinished->jobRunType = FOREGROUND;
                commandUnfinished->output = "";
                commandArgs = (char *) malloc(sizeof(char *) * MAX_ARG_LEN * MAX_ARGS);
                commandUnfinished->args = commandArgs;
                commandUnfinished->command = "";
                commandUnfinished->input = "";
                commandUnfinished->inputAppend = 0;
                commandUnfinished->jobRunType = FOREGROUND;
                commandUnfinished->output = "";
                continue;
            }
            commandArgs[argsq] = args[i];
            ++argsq;
            continue;

        } else { // Si es comando
            argsq = 0;
            commandUnfinished->args = commandArgs;
            commandArgs = (char *) malloc(sizeof(char *) * MAX_ARG_LEN * MAX_ARGS);
            ++commandsq;
        }
        int aux = 0;
        for (int j = 0; j < argq; j++) { // Ciclo para comparar con builtins

            if (strcmp(args[i], builtins[j]) == 0) {
                commandUnfinished->command = args[i];   
                ++commandsq;
                aux = 1;
                break;
            }
            
        }
        if (aux == 1) {continue;}
        if (strcmp(args[i], "&") == 0) {
            isPipe = 0;
            commandUnfinished->jobRunType = BACKGROUND;
            continue;

        }

    }
    Command commandFinished = (Command) malloc(sizeof(Command));
    char ** cargs = malloc(sizeof(commandUnfinished->args));
    copy_array(cargs, commandUnfinished->args, argsq);
    commandFinished->args = cargs;
    char * ccmd = malloc(sizeof(commandUnfinished->command));
    strcpy(ccmd, commandUnfinished->command);
    commandFinished->command = ccmd;
    char * cinput = malloc(sizeof(commandUnfinished->input));
    strcpy(cinput, commandUnfinished->input);
    commandFinished->input = cinput;
    char * coutput = malloc(sizeof(commandUnfinished->output));
    strcpy(coutput, commandUnfinished->output);
    commandFinished->output = coutput;

    if (commandUnfinished->input == 0) {
        commandFinished->input = 0;
    } else {commandFinished->input = 1;}
    if (commandUnfinished->jobRunType == BACKGROUND) {
        commandFinished->jobRunType = BACKGROUND;
    } else {commandFinished->jobRunType = FOREGROUND;}
    
    commands = realloc(commands, sizeof(commands) + sizeof(commandFinished));
    commands[commandsq][isPipe] = commandFinished; 

    commandUnfinished->command = "";
    commandUnfinished->input = "";
    commandUnfinished->inputAppend = 0;
    commandUnfinished->jobRunType = FOREGROUND;
    commandUnfinished->output = "";
    commandArgs = (char *) malloc(sizeof(char *) * MAX_ARG_LEN * MAX_ARGS);
    commandUnfinished->args = commandArgs;
    commandUnfinished->command = "";
    commandUnfinished->input = "";
    commandUnfinished->inputAppend = 0;
    commandUnfinished->jobRunType = FOREGROUND;
    commandUnfinished->output = "";
    
}