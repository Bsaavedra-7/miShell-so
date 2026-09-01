#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "shell.h"

int main(void) {
    
    while (1) {
        char line[MAX_LINE];
        printf("miShell> ");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break; 
        }
        
        line[strcspn(line, "\n")] = 0;

        printf("You entered: %s\n", line);
    }
    return 0;
}