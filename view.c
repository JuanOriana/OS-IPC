#define _XOPEN_SOURCE 500

#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

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
    else
    {
        scanf("%d", &fileCount);
    }

    sem_t *mutexSem, *fullSem;

    if ((mutexSem = sem_open(SEM_MUTEX_NAME, O_CREAT, 0660, 1)) == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    if ((fullSem = sem_open(SEM_FULL_NAME, O_CREAT, 0660, 0)) == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    int shmFd = shm_open(SHMEM_PATH, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR);
    char *shmBase = mmap(NULL, MAX_OUTPUT_SIZE * fileCount + sizeof(long), PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);

    while (1)
    {
        sem_wait(fullSem);
        sem_wait(mutexSem);
        printf("%s", shmBase + sizeof(long) + (*(long *)shmBase) * MAX_OUTPUT_SIZE);
        (*(long *)shmBase)--;
        sem_post(mutexSem);
    }

    close(shmFd);

    if (shm_unlink(SHMEM_PATH) < 0)
    {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink(SEM_MUTEX_NAME) < 0)
    {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink(SEM_FULL_NAME) < 0)
    {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }
    return 0;
}
