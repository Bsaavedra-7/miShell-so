#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shell.h"
#include "child.h"
#include <signal.h>
#include <unistd.h>
#include "validar.h"

void copy_array(char **to, char **from, int n) {
    for (int i = 0; i < n; i++) {
        to[i] = malloc(strlen(from[i]) + 1);
        strcpy(to[i], from[i]);
    }
}


// Separa argumentos en comandos y pipes de comandos
int sepCmds(char **args, struct Command *** commands, int argq) { // Diferencia comandos de argumentos
    const char* builtins[] = {"cd", "exit", "jobs", "pmon"};
    int builtinsq = 4;
    struct Command commandUnfinished; // para despues hacer malloc y añadirlo al array
    commandUnfinished.jobRunType = FOREGROUND;
    int commandsq = 0, endOfCommand = 0, isPipe = 0, skipLoop = 0, nextCommandisBackground = 0;
    int argsq = 0; // variable para contar argumentos de cada comando
    char **commandArgs = (char **) malloc(sizeof(char **) * MAX_ARGS);
    // inicializacion de instancia 
    commandUnfinished.args = commandArgs;
    commandUnfinished.command = "";
    commandUnfinished.input = "";
    commandUnfinished.inputAppend = 0;
    commandUnfinished.jobRunType = FOREGROUND;
    commandUnfinished.output = "";
    // 
    for (int i = 0; i < argq; i++) {
        if (skipLoop == 1) {skipLoop = 0; continue;}      
        if (!es_comando(args[i])) { // Si no es un comando
            if (strcmp(args[i], "|") == 0) {
                struct Command * commandFinished = (struct Command *) malloc(sizeof(struct Command) * MAX_COMMANDS_IN_LINE);
                commandFinished->output = malloc(1); // Para desacoplar output y args, por algun motivo se acoplaron solas??
                char ** cargs = (char**)malloc(sizeof(*commandUnfinished.args) * argsq);
                copy_array(cargs, commandUnfinished.args, argsq);
                commandFinished->args = cargs;
                char * ccmd = malloc(strlen(commandUnfinished.command) + 1);
                strcpy(ccmd, commandUnfinished.command);
                    commandFinished->command = ccmd;
                char * cinput = malloc(strlen(commandUnfinished.input) + 1);
                strcpy(cinput, commandUnfinished.input);
                commandFinished->input = cinput;
                char * coutput = malloc(strlen(commandUnfinished.output) + 1);
                strcpy(coutput, commandUnfinished.output);
                commandFinished->output = coutput;
                commandFinished->argsq = argsq;

                if (commandUnfinished.inputAppend == 0) {
                    commandFinished->inputAppend = 0;
                } else {commandFinished->inputAppend = 1;}
                if (commandUnfinished.jobRunType == BACKGROUND) {
                    commandFinished->jobRunType = BACKGROUND;
                } else {commandFinished->jobRunType = FOREGROUND;}
                
                commandFinished->pipe = 1;
                commands[commandsq - 1][isPipe] = commandFinished; 
                ++isPipe; 

                commandUnfinished.command = "";
                commandUnfinished.input = "";
                commandUnfinished.inputAppend = 0;
                commandUnfinished.jobRunType = FOREGROUND;
                commandUnfinished.output = "";
                commandArgs = (char **) malloc(sizeof(char **) * MAX_ARGS);
                argsq = 0;
                continue;
            } 
            if (strcmp(args[i], ">") == 0) {
                commandUnfinished.input = args[i + 1]; 
                commandUnfinished.inputAppend = 0;
                skipLoop = 1;
                continue;
            }
            if (strcmp(args[i], ">>") == 0) {
                commandUnfinished.input = args[i + 1];
                commandUnfinished.inputAppend = 1;
                skipLoop = 1;
                continue;
            }
            if (strcmp(args[i], "<") == 0) {
                commandUnfinished.output = args[i + 1];
                skipLoop = 1;
                continue;
            }
            if (strcmp(args[i], "&") == 0) {
                commandUnfinished.jobRunType = BACKGROUND;
                continue;

            }
            if (strcmp(args[i], "&&") == 0) {
                struct Command * commandFinished = (struct Command *) malloc(sizeof(struct Command));
                commandFinished->output = malloc(1);
                char ** cargs = (char**)malloc(sizeof(*commandUnfinished.args) * argsq);
                copy_array(cargs, commandUnfinished.args, argsq);
                commandFinished->args = cargs;
                char * ccmd = malloc(strlen(commandUnfinished.command) + 1);
                strcpy(ccmd, commandUnfinished.command);
                commandFinished->command = ccmd;
                char * cinput = malloc(strlen(commandUnfinished.input) + 1);
                strcpy(cinput, commandUnfinished.input);
                commandFinished->input = cinput;
                char * coutput = malloc(strlen(commandUnfinished.output) + 1);
                strcpy(coutput, commandUnfinished.output);
                commandFinished->output = coutput;
                commandFinished->argsq = argsq;
                argsq = 0;
                commandFinished->pipe = 0;

                if (commandUnfinished.inputAppend == 0) {
                    commandFinished->inputAppend = 0;
                } else {commandFinished->inputAppend = 1;}
                if (commandUnfinished.jobRunType == BACKGROUND) {
                    commandFinished->jobRunType = BACKGROUND;
                } else {commandFinished->jobRunType = FOREGROUND;}
                commands[commandsq - 1][isPipe] = commandFinished; 

                commandUnfinished.command = "";
                commandUnfinished.input = "";
                commandUnfinished.inputAppend = 0;
                commandUnfinished.jobRunType = FOREGROUND;
                commandUnfinished.output = "";
                commandArgs = (char **) malloc(sizeof(char **) * MAX_ARGS);
                commandUnfinished.args = commandArgs;
                commandUnfinished.command = "";
                commandUnfinished.input = "";
                commandUnfinished.inputAppend = 0;
                commandUnfinished.jobRunType = FOREGROUND;
                commandUnfinished.output = "";
                isPipe = 0;
                continue;
            }
            commandArgs[argsq] = args[i];
            commandUnfinished.args = commandArgs;
            ++argsq;
            commandUnfinished.argsq = argsq;
            continue;

        } else { // Si es comando
            argsq = 0;
            if (isPipe == 0) {
                commands[commandsq] =  (struct Command **) malloc(sizeof(struct Command*) * MAX_COMMANDS_IN_LINE);
                ++commandsq;
            } 
            if (strcmp(commandUnfinished.command, "") == 0) {
                commandUnfinished.command = args[i];
                commandArgs = (char **) malloc(sizeof(char **) * MAX_ARGS);
                commandUnfinished.args = commandArgs;
            } else {
                commandArgs[argsq] = args[i];
                ++argsq;
            }
            
        }
        if (strcmp(args[i], "&") == 0) {
            isPipe = 0;
            commandUnfinished.jobRunType = BACKGROUND;
            continue;

        }

    }
    struct Command * commandFinished = (struct Command *) malloc(sizeof(struct Command));
    commandFinished->output = malloc(1);
    char ** cargs = (char**)malloc(sizeof(*commandUnfinished.args) * argsq);
    copy_array(cargs, commandUnfinished.args, argsq);
    commandFinished->args = cargs;
    char * ccmd = malloc(strlen(commandUnfinished.command) + 1);
    strcpy(ccmd, commandUnfinished.command);
    commandFinished->command = ccmd;
    char * cinput = malloc(strlen(commandUnfinished.input) + 1);
    strcpy(cinput, commandUnfinished.input);
    commandFinished->input = cinput;
    char * coutput = malloc(strlen(commandUnfinished.output) + 1);
    strcpy(coutput, commandUnfinished.output);
    commandFinished->output = coutput;
    commandFinished->argsq = argsq;


    if (commandUnfinished.inputAppend == 0) {
        commandFinished->inputAppend = 0;
    } else {commandFinished->inputAppend = 1;}
    if (commandUnfinished.jobRunType == BACKGROUND) {
        commandFinished->jobRunType = BACKGROUND;
    } else {commandFinished->jobRunType = FOREGROUND;}
    commandFinished->pipe = 0;
    commands[commandsq - 1][isPipe] = commandFinished; 

    commandUnfinished.command = "";
    commandUnfinished.input = "";
    commandUnfinished.inputAppend = 0;
    commandUnfinished.jobRunType = FOREGROUND;
    commandUnfinished.output = "";
    commandArgs = (char **) malloc(sizeof(char **) * MAX_ARGS);
    commandUnfinished.args = commandArgs;
    commandUnfinished.command = "";
    commandUnfinished.input = "";
    commandUnfinished.inputAppend = 0;
    commandUnfinished.jobRunType = FOREGROUND;
    commandUnfinished.output = "";
    struct Command* a = commands[0][1];
    struct Command* b = commands[1][0];

    return commandsq;
    
}
