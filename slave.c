#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int readFile(char *line);

int main(int argc, char const *argv[])
{
    char str[256];
    char line[256];
    while (scanf("%s", str) != EOF)
    {
        if (strcmp(str, "") == 0)
            return 0;
        FILE *fd;
        if ((fd = fopen(str, "r")) == NULL)
        {
            perror("fopen");
            exit(-1);
        }
        fscanf(fd, "%[^\n]", line);
        printf("%d\n", readFile(line));
        fclose(fd);
    }

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
