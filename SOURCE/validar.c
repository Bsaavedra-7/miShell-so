
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>

// verificamos si la ruta corresponde a un archivo ejecutable
bool es_ejecutable(const char *ruta)
{
    struct stat info;

    return stat(ruta, &info) == 0 &&
           S_ISREG(info.st_mode) &&
           access(ruta, X_OK) == 0;
}

bool es_comando(const char *comando)
{
    // verificamos que el comando no este vacio
    if (comando == NULL || comando[0] == '\0')
        return false;

    // si tiene una ruta, verificamos directamente
    if (strchr(comando, '/') != NULL)
        return es_ejecutable(comando);

    // obtenemos las rutas donde Linux busca los comandos
    const char *path = getenv("PATH");

    if (path == NULL)
        return false;

    char *copia = strdup(path);

    if (copia == NULL)
        return false;

    char *resto = copia;
    char *directorio;

    // recorremos los directorios de PATH
    while ((directorio = strsep(&resto, ":")) != NULL)
    {
        // un directorio vacio representa el directorio actual
        if (directorio[0] == '\0')
            directorio = ".";

        size_t largo = strlen(directorio) +
                       strlen(comando) + 2;

        char *ruta = malloc(largo);

        if (ruta == NULL)
        {
            free(copia);
            return false;
        }

        snprintf(ruta, largo, "%s/%s",
                 directorio, comando);

        // si encontramos el ejecutable, retornamos true
        bool encontrado = es_ejecutable(ruta);

        free(ruta);

        if (encontrado)
        {
            free(copia);
            return true;
        }
    }

    free(copia);

    // no encontramos el comando
    return false;
}
