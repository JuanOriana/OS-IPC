
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
#include "errorHandling.h"

struct Resources
{
    char *shmBase;
    sem_t *mutexSem, *fullSem;
    int shmSize, shmFd;
    char *shmPath, *mutexPath, *fullPath;
};

ResourcesPtr resourcesInit(int shmSize, char *shmPath, char *mutexPath, char *fullPath)
{
    ResourcesPtr resources = (ResourcesPtr)malloc(sizeof(struct Resources));
    resources->shmSize = shmSize;
    resources->shmPath = shmPath;
    resources->mutexPath = mutexPath;
    resources->fullPath = fullPath;

    // NOTE: This unlinks may return with an ENOENT error if the files were previosuly created and not unlinked
    // However, its the only way to make sure that we will not open an unwanted file
    // and Alejo told us it was reasonable.
    shm_unlink(resources->shmPath);
    sem_unlink(resources->fullPath);
    sem_unlink(resources->mutexPath);

    //Sem opens with creation flag
    if ((resources->mutexSem = sem_open(mutexPath, O_CREAT | O_EXCL, 0660, 1)) == SEM_FAILED)
    {
        errorHandler("sem_open");
    }

    if ((resources->fullSem = sem_open(fullPath, O_CREAT | O_EXCL, 0660, 0)) == SEM_FAILED)
    {
        errorHandler("sem_open");
    }

    //Shm open with creation flag
    resources->shmFd = shm_open(shmPath, O_CREAT | O_RDWR | O_EXCL, S_IWUSR | S_IRUSR);

    if (resources->shmFd < 0)
    {
        errorHandler("shm_open");
    }

    // Size defs
    if (ftruncate(resources->shmFd, shmSize + sizeof(long)) < 0)
    {
        errorHandler("ftruncate");
    }

    // Mapping
    char *shmBase = mmap(NULL, shmSize + sizeof(long), PROT_READ | PROT_WRITE, MAP_SHARED, resources->shmFd, 0);

    if (shmBase == MAP_FAILED)
    {
        errorHandler("mmap");
    }

    *(long *)shmBase = 0;

    resources->shmBase = shmBase;

    return resources;
}

void resourcesUnlink(ResourcesPtr resources)
{
    if (munmap(resources->shmBase, resources->shmSize + sizeof(long)) < 0)
    {
        errorHandler("munmap");
    }

    if (shm_unlink(resources->shmPath) < 0)
    {
        errorHandler("shm_unlink");
    }

    if (sem_unlink(resources->fullPath) < 0)
    {
        errorHandler("sem_unlink");
    }

    if (sem_unlink(resources->mutexPath) < 0)
    {
        errorHandler("sem_unlink");
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
        errorHandler("sem_open");
    }

    if ((resources->fullSem = sem_open(fullPath, 0, 0660, 0)) == SEM_FAILED)
    {
        errorHandler("sem_open");
    }

    //CATCH ERRORS!!
    //Shm open WITHOUT creation flag
    resources->shmFd = shm_open(resources->shmPath, O_RDWR, S_IWUSR | S_IRUSR);

    if (resources->shmFd < 0)
    {
        errorHandler("shm_open");
    }

    //Mapping
    resources->shmBase = mmap(NULL, resources->shmSize + sizeof(long), PROT_READ | PROT_WRITE, MAP_SHARED, resources->shmFd, 0);

    if (resources->shmBase == MAP_FAILED)
    {
        errorHandler("mmap");
    }

    return resources;
}

void resourcesClose(ResourcesPtr resources)
{

    if (munmap(resources->shmBase, resources->shmSize + sizeof(long)) < 0)
    {
        errorHandler("munmap");
    }

    if (sem_close(resources->mutexSem) < 0)
    {
        errorHandler("sem_unlink");
    }

    if (sem_close(resources->fullSem) < 0)
    {
        errorHandler("sem_unlink");
    }

    if ((close(resources->shmFd)) < 0)
    {
        errorHandler("close");
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