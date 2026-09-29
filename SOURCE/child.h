#ifndef CHILD_H
#define CHILD_H
#include "shell.h"
#include <stdio.h>
#include "separateCommands.h"
#include <unistd.h>
#include <sys/types.h>
#include "shell.h"

void execute(struct Command *** commands, Job ** processes, int comQuant);
void job_add(pid_t *pids, Command ** pipeline, Job ** processes);

#endif