
#define _XOPEN_SOURCE 500

#include "resourcesADT.h"
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

struct Resources
{
    char *shmBase;
    sem_t *mutexSem, *fullSem;
    int shmSize, shmFd;
    char *shmPath, *mutexPath, *fullPath;
};

void errorHand(char *funcName)
{
    perror(funcName);
    exit(EXIT_FAILURE);
}

ResourcesPtr resourcesInit(int shmSize, char *shmPath, char *mutexPath, char *fullPath)
{
    ResourcesPtr resources = (ResourcesPtr)malloc(sizeof(struct Resources));
    resources->shmSize = shmSize;
    resources->shmPath = shmPath;
    resources->mutexPath = mutexPath;
    resources->fullPath = fullPath;

    //Sem opens with creation flag
    if ((resources->mutexSem = sem_open(mutexPath, O_CREAT | O_EXCL, 0660, 1)) == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    if ((resources->fullSem = sem_open(fullPath, O_CREAT | O_EXCL, 0660, 0)) == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    //Shm open with creation flag
    resources->shmFd = shm_open(shmPath, O_CREAT | O_RDWR | O_EXCL, S_IWUSR | S_IRUSR);

    if (resources->shmFd < 0)
    {
        errorHand("shm_open");
    }

    // Size defs
    if (ftruncate(resources->shmFd, shmSize + sizeof(long)) < 0)
    {
        errorHand("ftruncate");
    }

    // Mapping
    char *shmBase = mmap(NULL, shmSize + sizeof(long), PROT_READ | PROT_WRITE, MAP_SHARED, resources->shmFd, 0);

    if (shmBase == MAP_FAILED)
    {
        errorHand("mmap");
    }

    *(long *)shmBase = 0;

    resources->shmBase = shmBase;

    return resources;
}

void resourcesUnlink(ResourcesPtr resources)
{
    if (munmap(resources->shmBase, resources->shmSize + sizeof(long)) < 0)
    {
        errorHand("munmap");
    }

    if (shm_unlink(resources->shmPath) < 0)
    {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink(resources->fullPath) < 0)
    {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink(resources->mutexPath) < 0)
    {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }

    free(resources);
}

ResourcesPtr resourcesOpen(int shmSize, char *shmPath, char *mutexPath, char *fullPath)
{
    ResourcesPtr resources = (ResourcesPtr)malloc(sizeof(struct Resources));
    resources->shmSize = shmSize;
    resources->shmPath = shmPath;
    resources->mutexPath = mutexPath;
    resources->fullPath = fullPath;

    //Sem opens WITHOUT creation flag
    if ((resources->mutexSem = sem_open(mutexPath, 0, 0660, 1)) == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    if ((resources->fullSem = sem_open(fullPath, 0, 0660, 0)) == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    //CATCH ERRORS!!
    //Shm open WITHOUT creation flag
    resources->shmFd = shm_open(resources->shmPath, O_RDWR, S_IWUSR | S_IRUSR);

    if (resources->shmFd < 0)
    {
        errorHand("shm_open");
    }

    //Mapping
    resources->shmBase = mmap(NULL, resources->shmSize + sizeof(long), PROT_READ | PROT_WRITE, MAP_SHARED, resources->shmFd, 0);

    if (resources->shmBase == MAP_FAILED)
    {
        errorHand("mmap");
    }

    return resources;
}

void resourcesClose(ResourcesPtr resources)
{

    if (munmap(resources->shmBase, resources->shmSize + sizeof(long)) < 0)
    {
        errorHand("munmap");
    }

    if (sem_close(resources->mutexSem) < 0)
    {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }

    if (sem_close(resources->fullSem) < 0)
    {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }

    if ((close(resources->shmFd)) < 0)
    {
        errorHand("close");
    }

    free(resources);
}

sem_t *getMutex(ResourcesPtr resources)
{
    return resources->mutexSem;
}

sem_t *getFull(ResourcesPtr resources)
{
    return resources->fullSem;
}

char *getShmBase(ResourcesPtr resources)
{
    return resources->shmBase;
}