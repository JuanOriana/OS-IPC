#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFF_SIZE 1024

int readFile(char *line);

int main(int argc, char const *argv[])
{
    char *line = NULL;
    size_t len = 0;
    ssize_t lineSize = 0;

    while ((lineSize = getline(&line, &len, stdin)) > 0)
    {
        printf("%s", line);
    }
    free(line);
    return 0;
}

int readFile(char *line)
{
    char *token = strtok(line, " ");
    int count = 0;
    int sum = 0;
    while (token != NULL)
    {
        int aux = atoi(token);
        count++;
        if ((aux == 0 && strcmp(token, "0") != 0) || count >= 3)
        {
            printf("Error in parameters\n");
            exit(-1);
        }
        sum += aux;
        token = strtok(NULL, " ");
    }
    return sum;
}
