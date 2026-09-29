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
int sepCmds(char **args, Command *** commands, int argq) { // Diferencia comandos de argumentos
    const char* builtins[] = {"cd", "exit", "jobs", "pmon"};
    int builtinsq = 4;
    Command commandUnfinished; // para despues hacer malloc y añadirlo al array
    commandUnfinished.jobRunType = FOREGROUND;
    int commandsq = 0, endOfCommand = 0, isPipe = 0, skipLoop = 0, nextCommandisBackground = 0;
    int argsq = 0; // variable para contar argumentos de cada comando
    char **commandArgs = (char **) malloc(sizeof(char **) * MAX_ARGS);
    // inicializacion de instancia 
    commandUnfinished.args = commandArgs;
    commandUnfinished.command = "";
    commandUnfinished.infile = "";
    commandUnfinished.infileAppend = 0;
    commandUnfinished.jobRunType = FOREGROUND;
    commandUnfinished.outfile = "";
    // 
    for (int i = 0; i < argq; i++) {
        if (skipLoop == 1) {skipLoop = 0; continue;}      
        if (!es_comando(args[i])) { // Si no es un comando
            if (strcmp(args[i], "|") == 0) {
                Command * commandFinished = (Command *) malloc(sizeof(Command) * MAX_COMMANDS_IN_LINE);
                commandFinished->outfile = malloc(1); // Para desacoplar outfile y args, por algun motivo se acoplaron solas??
                char ** cargs = (char**)malloc(sizeof(*commandUnfinished.args) * (argsq + 1));
                cargs[argsq] = NULL;
                copy_array(cargs, commandUnfinished.args, argsq);
                commandFinished->args = cargs;
                char * ccmd = malloc(strlen(commandUnfinished.command) + 1);
                strcpy(ccmd, commandUnfinished.command);
                    commandFinished->command = ccmd;
                char * cinfile = malloc(strlen(commandUnfinished.infile) + 1);
                strcpy(cinfile, commandUnfinished.infile);
                commandFinished->infile = cinfile;
                char * coutfile = malloc(strlen(commandUnfinished.outfile) + 1);
                strcpy(coutfile, commandUnfinished.outfile);
                commandFinished->outfile = coutfile;
                commandFinished->argsq = argsq;

                if (commandUnfinished.infileAppend == 0) {
                    commandFinished->infileAppend = 0;
                } else {commandFinished->infileAppend = 1;}
                if (commandUnfinished.jobRunType == BACKGROUND) {
                    commandFinished->jobRunType = BACKGROUND;
                } else {commandFinished->jobRunType = FOREGROUND;}
                
                commandFinished->pipe = 1;
                commands[commandsq - 1][isPipe] = commandFinished; 
                ++isPipe; 

                commandUnfinished.command = "";
                commandUnfinished.infile = "";
                commandUnfinished.infileAppend = 0;
                commandUnfinished.jobRunType = FOREGROUND;
                commandUnfinished.outfile = "";
                commandArgs = (char **) malloc(sizeof(char **) * MAX_ARGS);
                argsq = 0;
                continue;
            } 
            if (strcmp(args[i], ">") == 0) {
                commandUnfinished.outfile = args[i + 1]; 
                commandUnfinished.infileAppend = 0;
                skipLoop = 1;
                continue;
            }
            if (strcmp(args[i], ">>") == 0) {
                commandUnfinished.infile = args[i + 1];
                commandUnfinished.infileAppend = 1;
                skipLoop = 1;
                continue;
            }
            if (strcmp(args[i], "<") == 0) {
                commandUnfinished.infile = args[i + 1];
                skipLoop = 1;
                continue;
            }
            if (strcmp(args[i], "&") == 0) {
                commandUnfinished.jobRunType = BACKGROUND;
                continue;

            }
            if (strcmp(args[i], "&&") == 0) {
                Command * commandFinished = (Command *) malloc(sizeof(Command));
                commandFinished->outfile = malloc(1);
                char ** cargs = (char**)malloc(sizeof(*commandUnfinished.args) * (argsq + 1));
                cargs[argsq] = NULL;
                copy_array(cargs, commandUnfinished.args, argsq);
                commandFinished->args = cargs;
                char * ccmd = malloc(strlen(commandUnfinished.command) + 1);
                strcpy(ccmd, commandUnfinished.command);
                commandFinished->command = ccmd;
                char * cinfile = malloc(strlen(commandUnfinished.infile) + 1);
                strcpy(cinfile, commandUnfinished.infile);
                commandFinished->infile = cinfile;
                char * coutfile = malloc(strlen(commandUnfinished.outfile) + 1);
                strcpy(coutfile, commandUnfinished.outfile);
                commandFinished->outfile = coutfile;
                commandFinished->argsq = argsq;
                argsq = 0;
                commandFinished->pipe = 0;

                if (commandUnfinished.infileAppend == 0) {
                    commandFinished->infileAppend = 0;
                } else {commandFinished->infileAppend = 1;}
                if (commandUnfinished.jobRunType == BACKGROUND) {
                    commandFinished->jobRunType = BACKGROUND;
                } else {commandFinished->jobRunType = FOREGROUND;}
                commands[commandsq - 1][isPipe] = commandFinished; 

                commandUnfinished.command = "";
                commandUnfinished.infile = "";
                commandUnfinished.infileAppend = 0;
                commandUnfinished.jobRunType = FOREGROUND;
                commandUnfinished.outfile = "";
                commandArgs = (char **) malloc(sizeof(char **) * MAX_ARGS);
                commandUnfinished.args = commandArgs;
                commandUnfinished.command = "";
                commandUnfinished.infile = "";
                commandUnfinished.infileAppend = 0;
                commandUnfinished.jobRunType = FOREGROUND;
                commandUnfinished.outfile = "";
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
                commands[commandsq] =  (Command **) malloc(sizeof(Command*) * MAX_COMMANDS_IN_LINE);
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
    Command * commandFinished = (Command *) malloc(sizeof(Command));
    commandFinished->outfile = malloc(1);
    char ** cargs = (char**)malloc(sizeof(*commandUnfinished.args) * (argsq + 1));
    cargs[argsq] = NULL;
    copy_array(cargs, commandUnfinished.args, argsq);
    commandFinished->args = cargs;
    char * ccmd = malloc(strlen(commandUnfinished.command) + 1);
    strcpy(ccmd, commandUnfinished.command);
    commandFinished->command = ccmd;
    char * cinfile = malloc(strlen(commandUnfinished.infile) + 1);
    strcpy(cinfile, commandUnfinished.infile);
    commandFinished->infile = cinfile;
    char * coutfile = malloc(strlen(commandUnfinished.outfile) + 1);
    strcpy(coutfile, commandUnfinished.outfile);
    commandFinished->outfile = coutfile;
    commandFinished->argsq = argsq;

    if (commandUnfinished.infileAppend == 0) {
        commandFinished->infileAppend = 0;
    } else {commandFinished->infileAppend = 1;}
    if (commandUnfinished.jobRunType == BACKGROUND) {
        commandFinished->jobRunType = BACKGROUND;
    } else {commandFinished->jobRunType = FOREGROUND;}
    commandFinished->pipe = 0;
    commands[commandsq - 1][isPipe] = commandFinished; 

    commandUnfinished.command = "";
    commandUnfinished.infile = "";
    commandUnfinished.infileAppend = 0;
    commandUnfinished.jobRunType = FOREGROUND;
    commandUnfinished.outfile = "";
    commandArgs = (char **) malloc(sizeof(char **) * MAX_ARGS);
    commandUnfinished.args = commandArgs;
    commandUnfinished.command = "";
    commandUnfinished.infile = "";
    commandUnfinished.infileAppend = 0;
    commandUnfinished.jobRunType = FOREGROUND;
    commandUnfinished.outfile = "";
    return commandsq;
    
}
