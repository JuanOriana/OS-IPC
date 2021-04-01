#define _XOPEN_SOURCE 500 //ftruncate warning

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <fcntl.h>

#define CHILD_COUNT 3
#define PIPES_PER_CHILD 2
#define FILEDESC_QTY 2
#define READ_END 0
#define WRITE_END 1
#define SLAVE_TO_MASTER 0
#define MASTER_TO_SLAVE 1
#define MAX_OUTPUT_SIZE 4096
#define SHMEM_PATH "/shmemBuffer"
#define BATCH_PERC 0.2
#define BUFF_SIZE 1024

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void errorHandler(char *funcName);
int initPipes(int pipeMat[][PIPES_PER_CHILD][FILEDESC_QTY], int pipeCount, int *maxFd);
int initForks(int *childIDs, int childCount, int pipes[][PIPES_PER_CHILD][FILEDESC_QTY]);
char *initShMem(int shmSize);
int waitAll(int *childIDs, int childCount);
int closePipes(int pipeCount, int pipes[][PIPES_PER_CHILD][FILEDESC_QTY]);
void buildReadSet(fd_set *set, int pipes[][2][2], char closedPipes[][2], int childCount);
void sendFile(int fd, const char *file, int fileLen);
void sendBatches(const char **files, int childCount, int batchSize, int pipes[][2][2], int *currIdx);

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <files>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // int totalFiles = argc - 1;
    // char *shmBase = initShMem(MAX_OUTPUT_SIZE * totalFiles);

    // printf("vista parameter is <param>\n");
    // sleep(2);

    pid_t childIDs[CHILD_COUNT];
    // 2 pipes per child
    int pipes[CHILD_COUNT][2][2];
    char closedPipes[CHILD_COUNT][2] = {{0}};

    int maxFd = -1;
    int fileCount = argc - 1;
    int childCount = MIN(CHILD_COUNT, fileCount);

    initPipes(pipes, childCount, &maxFd);
    initForks(childIDs, childCount, pipes);

    int currIdx = 1;
    int readSolves = 0;
    //Batches have to be of at least size 1
    int batchSize = MAX(fileCount * BATCH_PERC / childCount, 1);

    //Loading initial batches
    sendBatches(argv, childCount, batchSize, pipes, &currIdx);

    while (readSolves < fileCount)
    {
        char str[BUFF_SIZE] = {0};
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
                            if (closedPipes[i][WRITE_END] == 0)
                            {
                                close(pipes[i][MASTER_TO_SLAVE][WRITE_END]);
                                closedPipes[i][WRITE_END] = 1;
                            }
                        }
                        //(*(long *)shmBase)++;
                        //printf("%s\n", token);
                        token = strtok(NULL, "\n");
                        readSolves++;
                    }
                }
            }
        }
        printf("%d\n", readSolves);
    }

    waitAll(childIDs, childCount);
    //sleep(10); //Provisional para cat shmem y ver que este todo ok
    // if (shm_unlink(SHMEM_PATH) == -1)
    // {
    //     errorHandler("shm_unlink");
    // }

    // return 0;
}

int initPipes(int pipeMat[][PIPES_PER_CHILD][FILEDESC_QTY], int pipeCount, int *maxFd)
{

    for (int i = 0; i < pipeCount; i++)
    {
        for (int j = 0; j < PIPES_PER_CHILD; j++)
        {
            if (pipe(pipeMat[i][j]) < 0)
            {
                errorHandler("pipe");
            }

            if (pipeMat[i][j][READ_END] > *maxFd)
            {
                *maxFd = pipeMat[i][j][READ_END];
            }
        }
    }
    return 0;
}

int initForks(int *childIDs, int childCount, int pipes[][PIPES_PER_CHILD][FILEDESC_QTY])
{

    char *const *execParam = NULL;

    for (int i = 0; i < childCount; i++)
    {
        if ((childIDs[i] = fork()) < 0)
        {
            errorHandler("fork");
        }
        else if (childIDs[i] == 0)
        {
            dup2(pipes[i][SLAVE_TO_MASTER][WRITE_END], STDOUT_FILENO);
            dup2(pipes[i][MASTER_TO_SLAVE][READ_END], STDIN_FILENO);

            closePipes(childCount, pipes);
            execv("./slave", execParam);
            errorHandler("execv");
        }
        else
        {

            close(pipes[i][SLAVE_TO_MASTER][WRITE_END]);
            close(pipes[i][MASTER_TO_SLAVE][READ_END]);
        }
    }

    return 0;
}

char *initShMem(int shmSize)
{
    int shmFd = shm_open(SHMEM_PATH, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR);
    if (shmFd < 0)
    {
        errorHandler("shm_open");
    }

    if (ftruncate(shmFd, shmSize) < 0)
    {
        errorHandler("ftruncate");
    }

    char *shmBase = mmap(NULL, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);

    if (shmBase == MAP_FAILED)
    {
        errorHandler("mmap");
    }
    //  FD no longer needed to be open after mmap
    if ((close(shmFd)) < 0)
    {
        errorHandler("close");
    }
    *(long *)shmBase = 0;

    return shmBase;
}

int closePipes(int pipeCount, int pipes[][PIPES_PER_CHILD][FILEDESC_QTY])
{
    for (int i = 0; i < pipeCount; i++)
    {
        for (int j = 0; j < PIPES_PER_CHILD; j++)
        {
            for (int k = 0; k < FILEDESC_QTY; k++)
            {
                close(pipes[i][j][k]);
            }
        }
    }
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

void buildReadSet(fd_set *set, int pipes[][2][2], char closedPipes[][2], int childCount)
{
    FD_ZERO(set);
    for (int i = 0; i < childCount; i++)
    {
        if (!closedPipes[i][READ_END])
        {
            FD_SET(pipes[i][SLAVE_TO_MASTER][READ_END], set);
        }
    }
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

void errorHandler(char *funcName)
{
    perror(funcName);
    exit(EXIT_FAILURE);
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