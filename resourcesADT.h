#ifndef RESOURCES_H_
#define RESOURCES_H_

#include <semaphore.h>

typedef struct Resources *ResourcesPtr;

ResourcesPtr resourcesInit(int shmSize, char *shmPath, char *mutexPath, char *fullPath);
void resourcesUnlink(ResourcesPtr resources);
ResourcesPtr resourcesOpen(int shmSize, char *shmPath, char *mutexPath, char *fullPath);
void resourcesClose(ResourcesPtr resources);

sem_t *getMutex(ResourcesPtr resources);
sem_t *getFull(ResourcesPtr resources);
char *getShmBase(ResourcesPtr resources);

#endif