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

#define SHMEM_PATH "/shmemBuffer"
#define SEM_MUTEX_NAME "/sem-mutex"
#define SEM_FULL_NAME "/sem-full-count"
#define MAX_OUTPUT_SIZE 4096

int main(int argc, char const *argv[])
{
    int fileCount;
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
        if (scanf("%10d", &fileCount) != 1)
        {
            perror("scanf");
            exit(EXIT_FAILURE);
        }
        printf("%d\n", fileCount);
    }

    ResourcesPtr resources = resourcesOpen(fileCount * MAX_OUTPUT_SIZE, SHMEM_PATH, SEM_MUTEX_NAME, SEM_FULL_NAME);
    sem_t *fullSem = getFull(resources);
    sem_t *mutexSem = getMutex(resources);
    char *shmBase = getShmBase(resources);

    int i = 0;

    while (1)
    {
        if (i == fileCount)
        {
            break;
        }
        sem_wait(fullSem);
        sem_wait(mutexSem);
        if (strcmp(shmBase + sizeof(long) + (*(long *)shmBase) * MAX_OUTPUT_SIZE, "DONE") == 0)
        {
            sem_post(mutexSem);
            exit(0);
        }
        printf("%s", shmBase + sizeof(long) + (*(long *)shmBase) * MAX_OUTPUT_SIZE);
        (*(long *)shmBase)--;
        sem_post(mutexSem);
        i++;
    }

    resourcesClose(resources);

    return 0;
}
