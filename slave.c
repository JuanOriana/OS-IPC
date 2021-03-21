#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int readFile(char *line);

int main(int argc, char const *argv[]) {

    if (argc <= 1) {
        printf("Not enough parameters.\n");
        return -1;
    }
    for (int i = 1; i < argc; i++) {
        FILE *fd;
        if ((fd = fopen(argv[i], "r")) == NULL) {
            perror("fopen");
            exit(-1);
        }
        char line[256];
        fscanf(fd, "%[^\n]", line);
        printf("%d\n", readFile(line));
        fclose(fd);
    }
    return 0;
}

int readFile(char *line) {
    char *token = strtok(line, " ");
    int count = 0;
    int suma = 0;
    while (token != NULL) {
        int aux = atoi(token);
        count++;
        if ((aux == 0 && strcmp(token, "0") != 0)|| count >= 3) {
            printf("Error in parameters\n");
            exit(-1);
        }
        suma += aux;
        token = strtok(NULL, " ");
    }
    return suma;
}

