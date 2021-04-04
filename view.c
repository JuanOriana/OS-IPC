// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define _XOPEN_SOURCE 500

#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "resourcesADT.h"
#include "consts.h"
#include "errorHandling.h"

int main(int argc, char const *argv[])
{
    int fileCount = 0 ;
    if (argc > 2)
    {
        fprintf(stderr, "Usage: %s <fileCount>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    else if (argc == 2)
    {
        fileCount = atoi(argv[1]);
    }
    else if (argc == 1)
    {
        // Avoiding ridiculously large numbers as input
        if (scanf("%10d", &fileCount) != 1)
        {
            errorHandler("scanf");
        }
    }

    ResourcesPtr resources = resourcesOpen(fileCount * MAX_OUTPUT_SIZE, SHMEM_PATH, SEM_MUTEX_NAME, SEM_FULL_NAME);
    sem_t *fullSem = getFull(resources);
    sem_t *mutexSem = getMutex(resources);
    char *shmBase = getShmBase(resources);

    int i = 0;

    while (i < fileCount)
    {
        sem_wait(fullSem);
        sem_wait(mutexSem);
        printf("%s", shmBase + sizeof(long) + (*(long *)shmBase) * MAX_OUTPUT_SIZE);
        (*(long *)shmBase)--;
        sem_post(mutexSem);
        i++;
    }

    resourcesClose(resources);

    return 0;
}
