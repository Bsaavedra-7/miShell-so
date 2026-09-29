#include "executor.h"
#include "shell.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "child.h"

static void setup_child_io(int in_fd, int out_fd, Command *cmd);
static int execute_command(Command *cmd);
static char **build_argv(Command *cmd);

int execute_pipeline(struct Command **pipeline, int ncmds, Job **processes)
{
    pid_t pids[ncmds];
    int pipefd[2];
    int prev_fd = -1;
    
    // Determinar si la tubería entera corre en Background
    int background = (pipeline[ncmds - 1]->jobRunType == BACKGROUND);

    for (int i = 0; i < ncmds; i++)
    {
        Command *cmd = pipeline[i];
        int is_last = (i == ncmds - 1);

        if (!is_last)
        {
            if (pipe(pipefd) == -1)
            {
                perror("pipe");
                return -1;
            }
        }

        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");
            return -1;
        }

        // --- PROCESO HIJO ---
        if (pid == 0)
        {
            // R3: Ajustar señales en el proceso hijo
            if (!background) {
                // Si es Foreground, responde a Ctrl+C (SIGINT) y Ctrl+\ (SIGQUIT)
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
            } else {
                // R3: Si es Background, ignora Ctrl+C y Ctrl+\
                signal(SIGINT, SIG_IGN);
                signal(SIGQUIT, SIG_IGN);
            }

            if (!is_last) {
                close(pipefd[0]);
            }

            setup_child_io(prev_fd, is_last ? -1 : pipefd[1], cmd);
            execute_command(cmd);
        }

        // --- PROCESO PADRE ---
        pids[i] = pid;

        if (prev_fd != -1)
            close(prev_fd);
        if (!is_last)
            close(pipefd[1]);

        prev_fd = is_last ? -1 : pipefd[0];
    }

    if (prev_fd != -1)
        close(prev_fd);

    // R4: Procesamiento de ejecuciones en Background y Foreground
    if (background)
    {
        // Registrar en la tabla de trabajos de la shell
        job_add(pids, pipeline, processes);
        printf("[%d] %d\n", JOBS, pids[ncmds - 1]);
        return pids[ncmds - 1];
    }
    else
    {
        // Foreground: Esperar a que terminen todos los procesos de la tubería
        int status = 0;
        for (int i = 0; i < ncmds; i++)
        {
            waitpid(pids[i], &status, 0);
        }
        return WEXITSTATUS(status);
    }
}

int execute_command(Command *cmd){
    char **argv = build_argv(cmd);
    execvp(argv[0], argv);
    perror(argv[0]);
    _exit(127);
}

static void setup_child_io(int in_fd, int out_fd, Command *cmd)
{
    if (cmd->infile && cmd->infile[0] != '\0') {
        int fd = open(cmd->infile, O_RDONLY);
        if (fd < 0) { perror(cmd->infile); _exit(1); }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    else if (in_fd != -1) {
        dup2(in_fd, STDIN_FILENO);
        close(in_fd);
    }

    if (cmd->outfile && cmd->outfile[0] != '\0') {
        int flags = O_CREAT | O_WRONLY | (cmd->infileAppend ? O_APPEND : O_TRUNC);
        int fd = open(cmd->outfile, flags, 0644);
        if (fd < 0) { perror(cmd->outfile); _exit(1); }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    else if (out_fd != -1) {
        dup2(out_fd, STDOUT_FILENO);
        close(out_fd);
    }
}

static char **build_argv(Command *cmd) {
    char **argv = malloc(sizeof(char *) * (cmd->argsq + 2));
    argv[0] = cmd->command;                   
    for (int i = 0; i < cmd->argsq; i++) {
        argv[i + 1] = cmd->args[i];            
    }
    argv[cmd->argsq + 1] = NULL;               
    return argv;
}

void free_commands(struct Command **cmds, int ncmds) {
    for (int i = 0; i < ncmds; i++) {
        if (cmds[i]) {
            free(cmds[i]->command);
            free(cmds[i]->args);
            free(cmds[i]->infile);
            free(cmds[i]->outfile);
            free(cmds[i]);
        }
    }
    free(cmds);
}