#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <fcntl.h>

#define CHILD_COUNT 3
#define PIPES_PER_CHILD 2
#define FILEDESC_QTY 2
#define READ_END 0
#define WRITE_END 1
#define SLAVE_TO_MASTER 0
#define MASTER_TO_SLAVE 1

int initPipes(int pipeMat[][PIPES_PER_CHILD][FILEDESC_QTY], int pipeCount, int *maxFd);
int initForks(int *childIDs, int childCount, int pipes[][PIPES_PER_CHILD][FILEDESC_QTY]);
int waitAll(int *childIDs, int childCount);
int closePipes(int pipeCount, int pipes[][PIPES_PER_CHILD][FILEDESC_QTY]);
void buildReadSet(fd_set *set, int pipes[][2][2], char closedPipes[], int childCount);
void sendFile(int fd, const char *file, int fileLen);

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <files>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t childIDs[CHILD_COUNT];
    // 2 pipes per child
    int pipes[CHILD_COUNT][2][2];
    char closedPipes[CHILD_COUNT] = {0};

    int maxFd = -1;

    initPipes(pipes, CHILD_COUNT, &maxFd);
    initForks(childIDs, CHILD_COUNT, pipes);

    int fileCount = argc - 1;
    int currIdx = 1;
    int readSolves = 0;
    //CAMBIAR
    int batchSize = 1;

    //Loading initial batches CHANGE
    for (int i = 0; i < CHILD_COUNT; i++)
    {
        for (int j = 0; j < batchSize; j++)
        {

            sendFile(pipes[i][MASTER_TO_SLAVE][WRITE_END], argv[currIdx], strlen(argv[currIdx]));
            currIdx++;
        }
    }

    while (readSolves < fileCount)
    {
        char str[256] = {0};
        fd_set readSet;
        buildReadSet(&readSet, pipes, closedPipes, CHILD_COUNT);

        if (select(maxFd + 1, &readSet, NULL, NULL, NULL) <= 0)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < CHILD_COUNT; i++)
        {
            if (FD_ISSET(pipes[i][SLAVE_TO_MASTER][READ_END], &readSet))
            {
                if (read(pipes[i][SLAVE_TO_MASTER][READ_END], str, 256) == 0)
                {
                    closedPipes[i] = 1;
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
                            close(pipes[i][MASTER_TO_SLAVE][WRITE_END]);
                        }
                        printf("%s\n", token);
                        token = strtok(NULL, "\n");
                        readSolves++;
                    }
                }
            }
        }
    }

    waitAll(childIDs, CHILD_COUNT);

    return 0;
}

int initPipes(int pipeMat[][PIPES_PER_CHILD][FILEDESC_QTY], int pipeCount, int *maxFd)
{

    for (int i = 0; i < pipeCount; i++)
    {
        for (int j = 0; j < PIPES_PER_CHILD; j++)
        {
            if (pipe(pipeMat[i][j]) < 0)
            {
                perror("pipe");
                exit(EXIT_FAILURE);
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
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (childIDs[i] == 0)
        {
            dup2(pipes[i][SLAVE_TO_MASTER][WRITE_END], STDOUT_FILENO);
            dup2(pipes[i][MASTER_TO_SLAVE][READ_END], STDIN_FILENO);

            closePipes(CHILD_COUNT,pipes);
            execv("./slave", execParam);
            perror("execv");
            exit(EXIT_FAILURE);
        }
        else
        {

            close(pipes[i][SLAVE_TO_MASTER][WRITE_END]);
            close(pipes[i][MASTER_TO_SLAVE][READ_END]);
        }
    }

    return 0;
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
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}

void buildReadSet(fd_set *set, int pipes[][2][2], char closedPipes[], int childCount)
{
    FD_ZERO(set);
    for (int i = 0; i < childCount; i++)
    {
        if (!closedPipes[i])
        {
            FD_SET(pipes[i][SLAVE_TO_MASTER][READ_END], set);
        }
    }
}

void sendFile(int fd, const char *file, int fileLen)
{

    if (write(fd, file, fileLen) < 0)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
    if (write(fd, "\n", 1) < 0)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
}