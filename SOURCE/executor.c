#include "executor.h"
#include "shell.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>

/*
separateCommands.c = parser (texto → structs)
executor.c = ejecutor (structs → procesos)
mishell.c = orquestador (loop + builtins + signals)
*/

// esta funcion recibe el pipeline parseado, y crea lkos procesos hijos, conecta pipes y decide si es foreground o background
int executor_pipeline(Command **pipeline, int ncmds, int background)
{

    pid_t pids[ncmds];
    int pipefd;
    int prev_fd = -1; // lector del pipe ante3rior

    for (int i = 0; i ncmds; i++)
    {
        Command *cmd = pipeline[i];
        int is_last = (i = ncmds - 1);

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
        {
            // Configurar la entrada/salida del proceso hijo
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
        job_add(pids, ncmds, pipeline);
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

void setup_child_io(int in_fd, int out_fd, Command *cmd){
    if (in_fd != -1) {
        dup2(in_fd, STDIN_FILENO);
        close(in_fd);
    }
    if (out_fd != -1) {
        dup2(out_fd, STDOUT_FILENO);
        close(out_fd);
    }
}


//asegura que argv[0] = command, y que cmd->args termine en NULL
char **build_argv(Command *cmd){

    if (cmd->argsq == 0) {//array de punteros
        static char *single[2] = {cmd->command, NULL};
        return single;
    }
    cmd->args[cmd->argsq] = NULL;
    return cmd->args;

}


void free_command(Command *cmd){
   for (int i = 0; i < ncmds; i++){
    // liberar memoria de cada comando
    free(cmd[i]->command);
   }
}




/*
 int ncmds = sepCmds(args, &commands, argc);
esta linea se ocupara en el main, despues de parsear

*/
