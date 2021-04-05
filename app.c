// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define _XOPEN_SOURCE 500 //ftruncate warning

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include "resourcesADT.h"
#include "consts.h"
#include "libIPC.h"

#define CHILD_COUNT 3
#define BATCH_PERC 0.2

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int waitAll(int *childIDs, int childCount);
void sendFile(int fd, const char *file, int fileLen);
void sendBatches(const char **files, int childCount, int batchSize, int pipes[][2][2], int *currIdx);

int main(int argc, char const *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <files>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fileCount = argc - 1;
    int currIdx = 1;
    int readSolves = 0;

    // Resources inits
    ResourcesPtr resources = resourcesInit(fileCount * MAX_OUTPUT_SIZE, SHMEM_PATH, SEM_MUTEX_NAME, SEM_FULL_NAME);
    sem_t *fullSem = getFull(resources);
    sem_t *mutexSem = getMutex(resources);
    char *shmBase = getShmBase(resources);

    sleep(5);
    printf("%d\n", fileCount);

    // 2 pipes per child
    int pipes[CHILD_COUNT][2][2];
    pid_t childIDs[CHILD_COUNT];
    char closedPipes[CHILD_COUNT][2] = {{0}};

    // Can not have more childs than files
    int childCount = MIN(CHILD_COUNT, fileCount);

    // Pipe/ forks inits
    int maxFd = -1;
    initPipes(pipes, childCount, &maxFd);
    initForks(childIDs, childCount, pipes);

    //Batches have to be of at least size 1
    int batchSize = MAX(fileCount * BATCH_PERC / childCount, 1);

    FILE *answersFile = fopen("answers.txt", "w");

    if (answersFile == NULL)
    {
        errorHandler("fopen");
    }

    //Loading initial batches
    sendBatches(argv, childCount, batchSize, pipes, &currIdx);

    while (readSolves < fileCount)
    {
        fd_set readSet;
        buildReadSet(&readSet, pipes, closedPipes, childCount);

        if (select(maxFd + 1, &readSet, NULL, NULL, NULL) <= 0)
        {
            errorHandler("select");
        }

        for (int i = 0; i < childCount; i++)
        {
            if (FD_ISSET(pipes[i][SLAVE_TO_MASTER][READ_END], &readSet))
            {
                char str[BUFF_SIZE] = {0};
                if (read(pipes[i][SLAVE_TO_MASTER][READ_END], str, BUFF_SIZE) == 0)
                {
                    closedPipes[i][READ_END] = 1;
                    close(pipes[i][SLAVE_TO_MASTER][READ_END]);
                }
                else
                {
                    char *token = strtok(str, "\n");
                    while (token != NULL)
                    {
                        if (currIdx < argc)
                        {
                            sendFile(pipes[i][MASTER_TO_SLAVE][WRITE_END], argv[currIdx], strlen(argv[currIdx]));
                            currIdx++;
                        }
                        else
                        {
                            // Only close if it hasnt been previously closed
                            if (closedPipes[i][WRITE_END] == 0)
                            {
                                close(pipes[i][MASTER_TO_SLAVE][WRITE_END]);
                                closedPipes[i][WRITE_END] = 1;
                            }
                        }

                        // Shm access
                        sem_wait(mutexSem);
                        sprintf(shmBase + sizeof(long) + (*(long *)shmBase) * MAX_OUTPUT_SIZE, "%s\n", token);
                        (*(long *)shmBase)++;
                        sem_post(mutexSem);
                        sem_post(fullSem);

                        fprintf(answersFile, "%s\n", token);

                        token = strtok(NULL, "\n");
                        readSolves++;
                    }
                }
            }
        }
    }

    fclose(answersFile);

    waitAll(childIDs, childCount);
    resourcesUnlink(resources);

    return 0;
}

int waitAll(int *childIDs, int childCount)
{
    for (int i = 0; i < childCount; i++)
    {
        if (waitpid(childIDs[i], NULL, 0) < 0)
        {
            errorHandler("waitpid");
        }
    }
    return 0;
}

void sendFile(int fd, const char *file, int fileLen)
{

    if (write(fd, file, fileLen) < 0)
    {
        errorHandler("write");
    }
    if (write(fd, "\n", 1) < 0)
    {
        errorHandler("write");
    }
}

void sendBatches(const char **files, int childCount, int batchSize, int pipes[][2][2], int *currIdx)
{
    for (int i = 0; i < childCount; i++)
    {
        for (int j = 0; j < batchSize; j++)
        {

            sendFile(pipes[i][MASTER_TO_SLAVE][WRITE_END], files[*currIdx], strlen(files[*currIdx]));
            (*currIdx)++;
        }
    }
}