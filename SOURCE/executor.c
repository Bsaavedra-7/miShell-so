#include "executor.h"
#include "shell.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "child.h"

//para edtener o prevenir errores
static void setup_child_io(int in_fd, int out_fd, Command *cmd);
static int execute_command(Command *cmd);
static char **build_argv(Command *cmd);
/*
separateCommands.c = parser (texto → structs)
executor.c = ejecutor (structs → procesos)
mishell.c = orquestador (loop + builtins + signals)
*/

// esta funcion recibe el pipeline parseado, y crea lkos procesos hijos, conecta pipes y decide si es foreground o background
int execute_pipeline(struct Command **pipeline, int ncmds, Job **processes)
{

    pid_t pids[ncmds];
    int pipefd[2];
    int prev_fd = -1; // lector del pipe ante3rior
    int background;
    if (pipeline[ncmds - 1]->jobRunType == BACKGROUND) {
        background = 1;
    } else {background = 0;}

    for (int i = 0; i < ncmds; i++)
    {
        Command *cmd = pipeline[i];

        int is_last = (i == ncmds - 1);

        // si no es ultimo comando crear pipe
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
        // HIJO
        if (pid == 0)
        {//Restaurar SIGINT,SIGQUIT en foreground
            //osea SIG_DFL en foreground hace que ctrl+c y ctrl+\ maten al proceso hijo, en background no hace nada
                if (!background) {
                struct sigaction sa = {0};
                sa.sa_handler = SIG_DFL;
                sigaction(SIGINT, &sa, NULL);
                sigaction(SIGQUIT, &sa, NULL);
            }

            // cierra fds que no usa el proceso hijo
            if (prev_fd != -1) close(prev_fd);
            if (!is_last) {
                close(pipefd[0]);  // evita el deadlock, el hijo no lee del pipe
            }
            // pipefd[1]< se pasa a setup_child_io y se cierra ahí tras dup2

            setup_child_io(prev_fd, is_last ? -1 : pipefd[1], cmd);
            execute_command(cmd);
        }

        // PADRE
        pids[i] = pid;

        // Cerrar fds que ya no sirven en padre
        if (prev_fd != -1)
            close(prev_fd);
        if (!is_last)
            close(pipefd[1]); // cerrar extremo escritura

        prev_fd = is_last ? -1 : pipefd[0]; // guardar extremo lectura para siguiente
    }

    // Cerrar último fd de lectura en padre
    if (prev_fd != -1)
        close(prev_fd);

    if (background)
    {
        // Registrar job con pids y cmdline
        job_add(pids, pipeline, processes);
        return pids[ncmds - 1];
    }
    else
    {
        // Foreground: esperar todos
        int status = 0;
        for (int i = 0; i < ncmds; i++)
        {
            waitpid(pids[i], &status, 0);
        }
        return WEXITSTATUS(status);
    }
}

int execute_command(Command *cmd){
    char **argv = build_argv(cmd); // asegura argv[0]=command
    execvp(argv[0], argv);
    perror(argv[0]);
    _exit(127); // exec fallo
}

static void setup_child_io(int in_fd, int out_fd, Command *cmd)
{
    // redireccion entrada archivo <
    if (cmd->infile && cmd->infile[0] != '\0') {
        int fd = open(cmd->infile, O_RDONLY);// avbre archivo de lectura
        if (fd < 0) { perror(cmd->infile); _exit(1); }
        dup2(fd, STDIN_FILENO);// fd 0 : archivo de entrada
        close(fd);//cierra el fd del archivo, ya que dup2 lo duplica en fd 0
    }
    // 2 no hay archivo, entonces hay pipe anterior
    else if (in_fd != -1) {
        dup2(in_fd, STDIN_FILENO);
        close(in_fd);
    }

    // redireccion salida > o >>
    if (cmd->outfile && cmd->outfile[0] != '\0') {
        int flags = O_CREAT | O_WRONLY | (cmd->infileAppend ? O_APPEND : O_TRUNC);// abre archivo de salida, si infileAppend es 1, entonces es >>, sino es >
        int fd = open(cmd->outfile, flags, 0644);// abre archivo de salida
        if (fd < 0) { perror(cmd->outfile); _exit(1); }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    // pipe salida
    else if (out_fd != -1) {
        dup2(out_fd, STDOUT_FILENO);
        close(out_fd);
    }
}

static char **build_argv(Command *cmd) {
    char **argv = malloc(sizeof(char) * (cmd->argsq + 2));
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




/*
 int ncmds = sepCmds(args, &commands, argc);
esta linea se ocupara en el main, despues de parsear

*/
