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

        // Si shell debe poder ejecutar cmd1 && cmd2 habra que hacer esto dentro de un ciclo

        if (strcmp(args[0], "cd") == 0)
        { // Comando es cd

            if (chdir(args[1]) == -1)
            { // Cambia directorio, si falla retorna -1
                printf("ERROR AL CAMBIAR DIRECTORIO");
            }
            continue; // Salta al sgte ciclo IMPORTANTE
        }

        if (strcmp(args[0], "exit") == 0)
        {
            return 0;
        } // Cierra la shell

        if (strcmp(args[0], "jobs") == 0)
        { // TODO falta testear, no se si funcione en realidad

            for (int i = 0; i < MAX_JOBS; i++)
            {
                char *status = malloc(256), *cmd = malloc(256);
                strcpy(cmd, processes[i]->command); // TODO puede que tire error al 2do arg no ser const char *
                long pid = (long)processes[i]->job_id;
                switch (processes[i]->status)
                { // Unica forma que se de usar los nombres de los enum, quizas agregandoles un valor que sea su nombre??
                case RUNNING:
                    strcpy(status, "RUNNING");
                    break;
                case STOPPED:
                    strcpy(status, "STOPPED");
                    break;
                case TERMINATED:
                    strcpy(status, "TERMINATED");
                    break;
                default:
                    break;
                }

                printf("%ld, %s, *char", pid, cmd);
                free(status);
                free(cmd);
                continue;
            }
        }

        if (strcmp(args[0], "pmon") == 0)
        { 
            continue;
        }

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