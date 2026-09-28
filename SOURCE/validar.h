
#ifndef VALIDAR_H
#define VALIDAR_H

#include <stdbool.h>

// verificamos si el comando es interno de nuestra shell
bool es_interno(const char *comando);

// verificamos si es un comando valido
bool es_comando(const char *comando);

#endif

