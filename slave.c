#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFF_SIZE 1024

int readFile(char *line);

int main(int argc, char const *argv[])
{
    setvbuf(stdout,NULL,_IONBF,0);
    char *path = NULL;
    char line[256];
    size_t len = 0;
    ssize_t lineSize = 0;
    FILE* fd;

    while ((lineSize = getline(&path, &len, stdin)) > 0)
    {
        if(strcmp(path,"") == 0)
            return 0;

        path[strcspn(path, "\n")] = 0; //Clearing path

        if ((fd = fopen(path, "r")) == NULL)
        {
            perror("fopen");
            exit(-1);
        }

        fscanf(fd, "%[^\n]", line);
        printf("%d\n", readFile(line));
        fclose(fd);

    }
    free(path);
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
