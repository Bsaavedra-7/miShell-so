#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shell.h"
#include "child.h"
#include <signal.h>
#include <unistd.h>
#include "executor.h" 
#include "separateCommands.h"  

//  gcc mishell.c -o mishell
//  ./mishell

int tokenizar(char *line, char **args)
{

    line[strcspn(line, "\n")] = 0; // Eliminar el salto de linea

    // aca tokenizamos el comando, determinando espacios, tabs y salto de linea
    char *token = strtok(line, "\t\r\n ");

    int i = 0;
    // el ultimo arg tiene que ser null
    while (token != NULL && i < MAX_ARGS - 1)
    {
        args[i] = token;
        token = strtok(NULL, "\t\r\n ");
        i++;
    }

    args[i] = NULL;
    return i;
}

int JOBS = 0;

int main(void)
{
    
    char cwd[1024];
    Job *processes[MAX_JOBS] = {NULL};// por esto fallaba, no estaba inicializadp
    char *args[MAX_ARGS];

    while (1)
    {

        if (getcwd(cwd, sizeof(cwd)) == NULL)
        {
            perror("getcwd() error");
            return 1;
        }

        char line[MAX_LINE];
        printf("miShell:%s$ ", cwd);
        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            break;
        }

        line[strcspn(line, "\n")] = 0;

        int argc = tokenizar(line, args);
        struct Command **commands = malloc(sizeof(char*) * MAX_COMMANDS_IN_LINE); // Arreglo que contiene comandos, pipes vienen como arreglos con c/cmd, comandos solos vienen en un arreglo solos
        //  verificamos que el usuario haya ingresado un comando
        // si solo presiona Enter, volvemos a mostrar el prompt
        if (argc == 0)
        {
            continue;
        }
        int commandQuantity = sepCmds(args, commands, argc);

        // TODO: integrar separateCommands para separar los comandos
        // cuando existan pipes o redirecciones

        execute(commands, processes, commandQuantity);

        // si el comando es valido, creamos el proceso hijo
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");
            continue;
        }

        if (pid == 0)
        {
            // hijo: restaurar señales
            execvp(args[0], args);
            perror(args[0]); // solo llega si execvp falla
            exit(1);
        }
        
        else
        {
            // padre: registrar job si es background, waitpid si es foreground
            // (waitpid por ahora, & se implementa en R5)
            waitpid(pid, NULL, 0);
        }
    }

    return 0;
}