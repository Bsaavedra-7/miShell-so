#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "shell.h"
#include <unistd.h>
#include "separateCommands.h"
#include "validar.h" //  funciones para validar los comandos

// Funcion para agregar proceso a array de procesos
void job_add(pid_t pids, Command ** commands, Job ** processes) { 
    int i = 0;
    while (1) {
        processes[JOBS] = malloc(sizeof(Job));
        processes[JOBS]->command = commands[i]->command;
        processes[JOBS]->job_id = JOBS;
        processes[JOBS]->pid = pids;
        processes[JOBS]->status = RUNNING;
        ++JOBS;

        if (commands[i]->pipe == 0) { // Termina de recorrer array en elemento donde var pipe = 0, siempre este sera el ultimo elemento (o unico)
            break;
        }
        ++i;
    }

}

// Funcion para ejecutar comandos builtin y otros comandos
void execute(struct Command *** commands, Job ** processes, int comQuant) {
    for (int i = 0; i < comQuant; i++) {
        int j = 0;
        struct Command ** comms = commands[i];
        while (1) {
            struct Command * command = comms[j];
            if (strcmp(command->command, "cd") == 0) {
                if (strcmp(command->args[0], "") == 0) {
                    char * home = getenv("HOME");
                    if (home == NULL) {
                        printf("ERROR: No se encontro HOME\n");
                        continue;
                    }
                    if(chdir(home) == -1) {
                        perror("ERROR AL CAMBIAR DIRECTORIO");
                    }
                }
            }

            if (strcmp(command->command, "exit") == 0) {
                if (strcmp(command->args[0], "") != 0) {
                    return atoi(command->args[0]);
                }
                return 0;
            }

            if (command->pipe == 0) {break;}

            
            if (strcmp(command->args[0], "jobs") == 0) {
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
                printf("[%ld] %s %s\n", job_id, status, processes[i]->command);
            }
            continue;

            }

            if (strcmp(command->command, "pmon") == 0)
            { 
                // TODO: implementar monitor de procesos
                continue;
            }

            if (!es_comando(command->command)) {
                printf("Comando no enconrtrado: %s\n", command->command);
                continue;
            }

            if (JOBS == MAX_JOBS) {
                printf("Limite de procesos activos alcanzado : %d\n", MAX_JOBS);
                break;
            }

            ++j;
            if (j == comQuant) {break;}

        }

    }

}