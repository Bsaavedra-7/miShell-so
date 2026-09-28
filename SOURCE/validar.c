
#include <stdio.h>
#include <unistd.h>

void ejecutar_comando(char **args)
{
    // intentamos ejecutar el comando ingresado
    // execvp busca el programa en las rutas de PATH
    execvp(args[0], args);

    // si llegamos aca significa que execvp fallo
    perror(args[0]);

    // terminamos el proceso hijo con un codigo de error
    _exit(127);
}
