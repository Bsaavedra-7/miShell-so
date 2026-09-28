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

int sepCmds(char **args, struct Command *** commands, int argq) { // Diferencia comandos de argumentos
    const char* builtins[] = {"cd", "exit", "jobs", "pmon"};
    int builtinsq = 4;
    commands[0] = (struct Command **)malloc(sizeof(struct Command*));
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
                struct Command * commandFinished = (struct Command *) malloc(sizeof(struct Command *));
                char ** cargs = malloc(sizeof(commandUnfinished.args));
                copy_array(cargs, commandUnfinished.args, argsq);
                commandFinished->args = cargs;
                char * ccmd = malloc(sizeof(commandUnfinished.command));
                strcpy(ccmd, commandUnfinished.command);
                commandFinished->command = ccmd;
                char * cinput = malloc(sizeof(commandUnfinished.input));
                strcpy(cinput, commandUnfinished.input);
                commandFinished->input = cinput;
                char * coutput = malloc(sizeof(commandUnfinished.output));
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
                
                commands[commandsq - 1] =  realloc(commands[commandsq -1], sizeof(commands[commandsq - 1]) + sizeof(commandFinished));
                commands[commandsq][isPipe] = commandFinished; 
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
                argsq = 0;
                continue;
            }
            if (strcmp(args[i], ">>") == 0) {
                commandUnfinished.input = args[i + 1];
                commandUnfinished.inputAppend = 1;
                skipLoop = 1;
                argsq = 0;
                continue;
            }
            if (strcmp(args[i], "<") == 0) {
                commandUnfinished.output = args[i + 1];
                skipLoop = 1;
                argsq = 0;
                continue;
            }
            if (strcmp(args[i], "&") == 0) {
                commandUnfinished.jobRunType = BACKGROUND;
                continue;

            }
            if (strcmp(args[i], "&&") == 0) {

                struct Command * commandFinished = (struct Command *) malloc(sizeof(struct Command));
                char ** cargs = malloc(sizeof(commandUnfinished.args));
                copy_array(cargs, commandUnfinished.args, argsq);
                commandFinished->args = cargs;
                char * ccmd = malloc(sizeof(commandUnfinished.command));
                strcpy(ccmd, commandUnfinished.command);
                commandFinished->command = ccmd;
                char * cinput = malloc(sizeof(commandUnfinished.input));
                strcpy(cinput, commandUnfinished.input);
                commandFinished->input = cinput;
                char * coutput = malloc(sizeof(commandUnfinished.output));
                strcpy(coutput, commandUnfinished.output);
                commandFinished->output = coutput;
                commandFinished->argsq = argsq;
                argsq = 0;

                if (isPipe != 0) {commandFinished->pipe = 1;} 
                    else {commandFinished->pipe = 0;}

                if (commandUnfinished.inputAppend == 0) {
                    commandFinished->inputAppend = 0;
                } else {commandFinished->inputAppend = 1;}
                if (commandUnfinished.jobRunType == BACKGROUND) {
                    commandFinished->jobRunType = BACKGROUND;
                } else {commandFinished->jobRunType = FOREGROUND;}
                
                commands = realloc(commands, sizeof(commands) + sizeof(commandFinished));
                commands[commandsq][isPipe] = commandFinished; 

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
                continue;
            }
            commandArgs[argsq] = args[i];
            ++argsq;
            continue;

        } else { // Si es comando
            argsq = 0;
            commandUnfinished.args = commandArgs;
            commandArgs = (char **) malloc(sizeof(char **) * MAX_ARGS);
            ++commandsq;
        }
        if (strcmp(args[i], "&") == 0) {
            isPipe = 0;
            commandUnfinished.jobRunType = BACKGROUND;
            continue;

        }

    }
    struct Command * commandFinished = (struct Command *) malloc(sizeof(struct Command*));
    char ** cargs = malloc(sizeof(commandUnfinished.args));
    copy_array(cargs, commandUnfinished.args, argsq);
    commandFinished->args = cargs;
    char * ccmd = malloc(sizeof(commandUnfinished.command));
    strcpy(ccmd, commandUnfinished.command);
    commandFinished->command = ccmd;
    char * cinput = malloc(sizeof(commandUnfinished.input));
    strcpy(cinput, commandUnfinished.input);
    commandFinished->input = cinput;
    char * coutput = malloc(sizeof(commandUnfinished.output));
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
    commands = realloc(commands, sizeof(commands) + sizeof(commandFinished));
    commands[commandsq][isPipe] = commandFinished; 

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
    
}