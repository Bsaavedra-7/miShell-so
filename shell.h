#ifndef SHELL_H
#define SHELL_H

#include <sys/types.h>


#define MAX_JOBS 100//maximo de trabajos que se pueden ejecutar al mismo tiempo
#define MAX_ARGS 64//maximo de argumentos que puede tener un comando
#define MAX_LINE 1024//maximo de caracteres que puede tener el comando



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
}Job;


#endif