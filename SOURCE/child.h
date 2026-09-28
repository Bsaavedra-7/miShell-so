#ifndef CHILD_H
#define CHILD_H
#include "shell.h"
#include <stdio.h>
#include "separateCommands.h"
#include <unistd.h>
#include "shell.h"

void execute(struct Command *** commands, Job ** processes, int comQuant);

#endif