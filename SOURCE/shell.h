#ifndef SHELL_H
#define SHELL_H

#include <sys/types.h>
#include <stdio.h>


#define MAX_JOBS 100 // maximo de trabajos que se pueden ejecutar al mismo tiempo
#define MAX_ARGS 64 // maximo de argumentos que puede tener un comando
#define MAX_LINE 1024 // maximo de caracteres que puede tener el comando

#define PIPE_READ 0 // Para uso en pipes creadas ej ptc[PIPE_READ] (parent to child) sera para leer contenido desde el padre al hijo
#define PIPE_WRITE 1 // Para uso en pipes creadas, ver ej arriba

extern int JOBS; // Variable global para saber cuantos procesos hay

//enum para representar el estado de un trabajo
typedef enum {
    RUNNING,
    STOPPED,
    TERMINATED
}JobStatus;


//Estructura para representar un trabajo
typedef struct {
    int job_id;
    pid_t pid;
    char *command;
    JobStatus status;
    int ptc[2]; // pipe para comunicacion padre a hijo (podria servir?)
    int ctp[2]; // pipe para comunicacion hijo a padre (podria servir?)
}Job;


#endif