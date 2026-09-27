#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shell.h"
#include "child.h"
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

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
        (void)argc; // Evita el warning de "unused variable" temporalmente
        if (args[0] == NULL) {
            continue;
        }

        // Si shell debe poder ejecutar cmd1 && cmd2 habra que hacer esto dentro de un ciclo

        if (strcmp(args[0], "cd") == 0)
                { // Comando es cd

                    if (args[1] == NULL)
                    { 
                        // 1. Sin argumentos: va al directorio $HOME (exigido por la pauta)
                        char *home = getenv("HOME");
                        if (home != NULL)
                        {
                            chdir(home);
                        }
                    }
                    else
                    {
                        // 2. Con argumento: cambia a la ruta indicada
                        if (chdir(args[1]) != 0)
                        { 
                            // Muestra el error estándar del sistema (ej: "cd: No tal archivo o el directorio")
                            perror("cd");
                        }
                    }
                    continue; // Salta al sgte ciclo IMPORTANTE
                }

        if (strcmp(args[0], "exit") == 0)
        {
            int exit_code = 0;
            if (args[1] != NULL)
            {
                exit_code = atoi(args[1]);
            }
            exit(exit_code);
        }

        if (strcmp(args[0], "jobs") == 0)
                {
                    for (int i = 0; i < MAX_JOBS; i++)
                    {
                        // 1. Solo procesar las casillas que realmente tengan un proceso registrado
                        if (processes[i] != NULL)
                        {
                            char status[32] = "UNKNOWN";

                            // 2. Convertir el enum a texto para imprimirlo
                            switch (processes[i]->status)
                            {
                            case RUNNING:
                                strcpy(status, "RUNNING");
                                break;
                            case STOPPED:
                                strcpy(status, "STOPPED");
                                break;
                            case TERMINATED:
                                strcpy(status, "TERMINATED");
                                break;
                            }

                            // 3. Imprimir la información del trabajo directamente sin usar malloc
                            printf("[%d] PID: %ld | Estado: %s | Comando: %s\n", 
                                i + 1, (long)processes[i]->job_id, status, processes[i]->command);
                        }
                    }
                    continue; // Salta al siguiente ciclo del while(1) para no hacer fork()
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
            // --- MANEJO DE REDIRECCIONES (>, >>, <) ---
            for (int i = 0; args[i] != NULL; i++)
            {
                if (strcmp(args[i], ">") == 0)
                {
                    int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd < 0)
                    {
                        perror("open salida");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    args[i] = NULL;
                    break;
                }
                else if (strcmp(args[i], ">>") == 0)
                {
                    int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                    if (fd < 0)
                    {
                        perror("open salida append");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    args[i] = NULL;
                    break;
                }
                else if (strcmp(args[i], "<") == 0)
                {
                    int fd = open(args[i + 1], O_RDONLY);
                    if (fd < 0)
                    {
                        perror("open entrada");
                        exit(1);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                    args[i] = NULL;
                    break;
                }
            }

            // Ejecuta el comando limpio
            execvp(args[0], args);
            perror(args[0]); // Solo llega aquí si execvp falla
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