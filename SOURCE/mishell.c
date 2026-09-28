#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> //  necesario para utilizar waitpid()
#include "shell.h"
#include "child.h"
#include <signal.h>
#include <unistd.h>
#include "separateCommands.h"
#include "validar.h" //  funciones para validar los comandos

// gcc -Wall -Wextra -std=gnu11 -o mishell mishell.c validar.c
// ./mishell

int tokenizar(char *line, char **args)
{

    line[strcspn(line, "\n")] = 0; // Eliminar el salto de linea

    // aca tokenizamos el comando, determinando espacios, tabs y salto de linea
    char *token = strtok(line, "\t\r\n ");

    int i = 0;

    // el ultimo arg tiene que ser NULL
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

    // inicializamos el arreglo de procesos para evitar acceder a memoria invalida
    Job *processes[MAX_JOBS] = {NULL};

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
        fflush(stdout);

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

        // Reemplazar llamados a comandos internos abajo

        if (strcmp(args[0], "cd") == 0)
        {
            //  si no se ingresa un directorio, usamos HOME
            // esto evita que chdir reciba un argumento NULL
            char *directorio = args[1];

            if (directorio == NULL)
            {
                directorio = getenv("HOME");
            }

            if (directorio == NULL)
            {
                printf("ERROR: No se encontro HOME\n");
                continue;
            }

            // cambiamos el directorio de trabajo de la shell
            if (chdir(directorio) == -1)
            {
                perror("ERROR AL CAMBIAR DIRECTORIO");
            }

            continue; // Salta al sgte ciclo IMPORTANTE
        }

        if (strcmp(args[0], "exit") == 0)
        {
            //  permitimos ingresar un codigo de salida
            // si no se ingresa ninguno, retornamos 0
            if (args[1] != NULL)
            {
                return atoi(args[1]);
            }

            return 0;
        }

        if (strcmp(args[0], "jobs") == 0)
        {
            // TODO: registrar los procesos en background

            for (int i = 0; i < MAX_JOBS; i++)
            {
                //  verificamos que exista un proceso
                // antes de acceder a sus datos
                if (processes[i] == NULL)
                {
                    continue;
                }

                // determinamos el estado del proceso registrado
                const char *status = "UNKNOWN";

                switch (processes[i]->status)
                {
                case RUNNING:
                        status = "RUNNING";
                    break;

                case STOPPED:
                        status = "STOPPED";
                    break;

                case TERMINATED:
                        status = "TERMINATED";
                    break;

                default:
                    break;
                }

                long job_id = (long)processes[i]->job_id;

                //  mostramos el estado sin reservar memoria
                // ya no necesitamos utilizar malloc ni free
                printf("[%ld] %s %s\n",
                       job_id,
                       status,
                       processes[i]->command);
            }

            //  evitamos que jobs se ejecute como comando externo
            continue;
        }

        if (strcmp(args[0], "pmon") == 0)
        { 
            // TODO: implementar monitor de procesos
            continue;
        }

        //  verificamos si el comando existe en Linux
        // es_comando retorna true si encuentra un ejecutable valido
        // tambien reconoce los comandos internos de nuestra shell
        if (!es_comando(args[0]))
        {
            printf("Comando no encontrado: %s\n", args[0]);

            // si no existe, no necesitamos crear un proceso hijo
            continue;
        }

        // si el comando es valido, creamos el proceso hijo
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");
            continue;
        }

        if (pid == 0)
        {
            // hijo: ejecutar el comando ingresado

            // TODO: restaurar las señales del hijo cuando
            // se implemente el manejo de SIGINT y SIGQUIT

            // execvp busca y ejecuta el programa en el proceso hijo
            execvp(args[0], args);

            // solo llegamos aca si execvp falla
            //  aunque la validacion sea correcta,
            // el programa podria dejar de estar disponible
            perror(args[0]);

            // terminamos el hijo sin cerrar nuestra shell
            _exit(127);
        }
        else
        {
            // padre: registrar job si es background,
            // waitpid si es foreground

            // TODO: implementar background cuando se encuentre &
            // por ahora esperamos a que termine el proceso hijo

            if (waitpid(pid, NULL, 0) == -1)
            {
                perror("waitpid");
            }
        }
    }

    return 0;
}
