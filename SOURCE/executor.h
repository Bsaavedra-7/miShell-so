#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "shell.h"   // Para Command, JobRunType, MAX_ARGS
#include <sys/types.h> // pid_t



int execute_pipeline(struct Command **pipeline, int ncmds, Job **processes);

//libera la memoria
void free_commands(struct Command **cmds, int ncmds);

#endif