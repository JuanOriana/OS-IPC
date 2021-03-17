#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <files>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
//

    // DIR *pDir = opendir(argv[1]);
    // if (pDir == NULL)
    // {
    //     fprintf(stderr, "Can not open directory %s\n", argv[1]);
    //     exit(EXIT_FAILURE);
    // }

    // closedir(pDir);
    return 0;
}
