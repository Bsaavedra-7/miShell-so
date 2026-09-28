
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include "validar.h"

// verificamos si el comando es interno de nuestra shell
bool es_interno(const char *comando)
{
    if (comando == NULL)
        return false;

    // estos comandos los implementamos nosotros
    return strcmp(comando, "cd") == 0 ||
           strcmp(comando, "exit") == 0 ||
           strcmp(comando, "jobs") == 0 ||
           strcmp(comando, "pmon") == 0;
}

// verificamos si existe el archivo y tiene permiso de ejecucion
static bool es_ejecutable(const char *ruta)
{
    struct stat info;

    // comprobamos que sea un archivo regular ejecutable
    return stat(ruta, &info) == 0 &&
           S_ISREG(info.st_mode) &&
           access(ruta, X_OK) == 0;
}

// verificamos si el comando ingresado es valido
bool es_comando(const char *comando)
{
    // verificamos que el comando no este vacio
    if (comando == NULL || comando[0] == '\0')
        return false;

    // primero verificamos nuestros comandos internos
    if (es_interno(comando))
        return true;

    // si tiene una ruta, verificamos directamente
    if (strchr(comando, '/') != NULL)
        return es_ejecutable(comando);

    // obtenemos las rutas donde Linux busca los comandos
    const char *path = getenv("PATH");

    if (path == NULL)
        return false;

    const char *inicio = path;

    // recorremos cada directorio de PATH
    while (1)
    {
        const char *fin = strchr(inicio, ':');

        size_t largo_dir = fin
            ? (size_t)(fin - inicio)
            : strlen(inicio);

        // reservamos memoria para la ruta completa
        size_t largo = largo_dir + strlen(comando) + 3;

        char *ruta = malloc(largo);

        if (ruta == NULL)
            return false;

        if (largo_dir == 0)
        {
            // una entrada vacia representa el directorio actual
            snprintf(ruta, largo, "./%s", comando);
        }
        else
        {
            // construimos la ruta del posible ejecutable
            memcpy(ruta, inicio, largo_dir);
            ruta[largo_dir] = '/';
            strcpy(ruta + largo_dir + 1, comando);
        }

        // verificamos si encontramos el ejecutable
        bool encontrado = es_ejecutable(ruta);

        free(ruta);

        if (encontrado)
            return true;

        // si no quedan directorios, terminamos la busqueda
        if (fin == NULL)
            break;

        inicio = fin + 1;
    }

    // no encontramos el comando
    return false;
}
