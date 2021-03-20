#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
    if (argc <= 1) {
        printf("Error cantidad de parametros.\n");
        return -1;
    }
    for (int i = 1; i < argc; i++) {
        FILE *fd;
        char c[256];
        if ((fd = fopen(argv[i], "r")) == NULL) {
            perror("fopen");
            abort();
        }
        fscanf(fd, "%[^\n]", c);
        printf("Data from file %s: %s\n", argv[i], c);
        fclose(fd);
    }
    return 0;
}
