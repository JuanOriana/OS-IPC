#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define CHILD_COUNT 3
#define READ_END 0
#define WRITE_END 1

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <files>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fileCount = argc - 1;
    pid_t childIDs[CHILD_COUNT];
    int pipes[CHILD_COUNT][2];

    for (int i = 0; i < CHILD_COUNT; i++)
    {
        if (pipe(pipes[i]) < 0)
        {
            perror("Unsuccesful pipe");
            abort();
        }
    }

    for (i = 0; i < CHILD_COUNT; i++)
    {
        if ((childIDs[i] = fork()) < 0)
        {
            perror("Unsuccesful fork");
            abort();
        }
        else if (childIDs[i] == 0)
        {
            execv("./slave", NULL);
        }
    }

    for (i = 0; i < CHILD_COUNT; i++)
    {
        if (waitpid(childIDs[i], NULL, 0) < 0)
        {
            perror("Unsuccesful wait");
            abort();
        }
    }

    return 0;
}
