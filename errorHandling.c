#include <stdio.h>
#include <stdlib.h>

void errorHandler(char *funcName)
{
    perror(funcName);
    exit(EXIT_FAILURE);
}