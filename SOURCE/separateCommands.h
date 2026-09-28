#ifndef SEPARATECOMMANDS_H
#define SEPARATECOMMANDS_H 
#include "shell.h"
#include <stdio.h>

int sepCmds(char **args, struct Command *** commands, int argq);

#endif