#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> //  necesario para utilizar waitpid()
#include "shell.h"
#include "child.h"
#include <signal.h>
#include <unistd.h>
#include "separateCommands.h"
#include "validar.h" //  funciones para validar los comandos

void printcmd(struct Command command) {
    printf("%s\n", command.command);
    for (int i = 0; i < command.argsq; i++) {
        printf("%s\n", command.args[i]);
    }
    printf("%s\n", command.input);
    printf("%d\n", command.inputAppend);
    printf("%d\n", command.output);
    if (command.jobRunType == BACKGROUND) {printf("BACKGROUND\n");}
    else {printf("FOREGROUND");}
    printf("%d\n%d\n", command.argsq, command.pipe);

}

int main() {
    char **args;
    struct Command ** commands = malloc(sizeof(struct Command**));
    int argq = 8;
    char a[8][20] = {"cd", "wea", "|", "ls", "lol", "&&", "rm", "we1"};
    for (int i = 0; i < argq; i++) {
        strcpy(args[i], a[i]);
    }
    int cmdq = sepCmds(args, &commands, argq);

    for (int i = 0; i < cmdq; i++) {
        int j = 0;
        while(1) {
            printcmd(commands[i][j]);
            if (commands[i][j].pipe == 0) {break;}
        }

    }

}