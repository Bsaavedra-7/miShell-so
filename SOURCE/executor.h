#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "shell.h"   // Para Command, JobRunType, MAX_ARGS
#include <sys/types.h> // pid_t


int execute_pipeline(Command **pipeline, int ncmds, int background);

//libera la memoria
void free_commands(Command **cmds, int ncmds);

#endif