#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *fDir = fopen(argv[1], "w");
    if (fDir == NULL)
    {
        fprintf(stderr, "Can not open directory %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    fclose(fDir);
    return 0;
}
