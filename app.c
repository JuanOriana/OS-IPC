#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define CHILD_COUNT 3
#define PIPES_QTY 2
#define FILEDESC_QTY 2
#define READ_END 0
#define WRITE_END 1
#define SLAVE_TO_MASTER 0
#define MASTER_TO_SLAVE 1

int initPipes(int pipeMat[][PIPES_QTY][FILEDESC_QTY], int pipeCount);
int initForks(int *childIDs, int childCount, int pipes[][PIPES_QTY][FILEDESC_QTY]);
int waitAll(int *childIDs, int childCount);
int closeUnrelatedPipes(int importantIdx, int pipeCount, int pipes[][PIPES_QTY][FILEDESC_QTY]);

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

    initPipes(pipes, CHILD_COUNT);
    initForks(childIDs, CHILD_COUNT, pipes);

    int fileCount = argc - 1;
    int currIdx = 1;
    int batchSize = fileCount / CHILD_COUNT;

    //Loading initial batches
    for (int i = 0; i < CHILD_COUNT; i++)
    {
        for (int j = 0; j < batchSize; j++)
        {

            if (write(pipes[i][MASTER_TO_SLAVE][WRITE_END], argv[currIdx], strlen(argv[currIdx])) < 0)
            {
                perror("Error writing in child");
                exit(EXIT_FAILURE);
            }
            if (write(pipes[i][MASTER_TO_SLAVE][WRITE_END], " ", 2) < 0)
            {
                perror("Error writing in child");
                exit(EXIT_FAILURE);
            }
            currIdx++;
        }
        //TODO: REMOVER CUANDO IMPLEMENTEMOS BIEN LOS BATCHES
        write(pipes[i][MASTER_TO_SLAVE][WRITE_END], " ", 2);
        close(pipes[i][MASTER_TO_SLAVE][WRITE_END]);
    }

    waitAll(childIDs, CHILD_COUNT);

    return 0;
}



int initPipes(int pipeMat[][PIPES_QTY][FILEDESC_QTY], int pipeCount)
{

    for (int i = 0; i < pipeCount; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            if (pipe(pipeMat[i][j]) < 0)
            {
                perror("Unsuccesful pipe");
                exit(EXIT_FAILURE);
            }
        }
    }
    return 0;
}

int initForks(int *childIDs, int childCount, int pipes[][PIPES_QTY][FILEDESC_QTY])
{

    char *const *execParam = NULL;

    for (int i = 0; i < childCount; i++)
    {
        if ((childIDs[i] = fork()) < 0)
        {
            perror("Unsuccesful fork");
            exit(EXIT_FAILURE);
        }
        else if (childIDs[i] == 0)
        {
            // dup2(pipes[i][SLAVE_TO_MASTER][WRITE_END], STDOUT_FILENO);
            dup2(pipes[i][MASTER_TO_SLAVE][READ_END], STDIN_FILENO);

            close(pipes[i][SLAVE_TO_MASTER][READ_END]);
            close(pipes[i][SLAVE_TO_MASTER][WRITE_END]);
            close(pipes[i][MASTER_TO_SLAVE][READ_END]);
            close(pipes[i][MASTER_TO_SLAVE][WRITE_END]);
            closeUnrelatedPipes(i, childCount, pipes);

            execv("./slave", execParam);
            perror("Exec failure");
            exit(EXIT_FAILURE);
        }
        else
        {

            close(pipes[i][SLAVE_TO_MASTER][READ_END]);
            close(pipes[i][SLAVE_TO_MASTER][WRITE_END]);
            close(pipes[i][MASTER_TO_SLAVE][READ_END]);
        }
    }

    return 0;
}

int closeUnrelatedPipes(int importantIdx, int pipeCount, int pipes[][PIPES_QTY][FILEDESC_QTY])
{
    for (int i = 0; i < pipeCount ; i++)
    {
        if (i != importantIdx)
        {
            for (int j = 0; j < PIPES_QTY; j++)
            {
                for (int k = 0; k < FILEDESC_QTY; k++)
                {
                    close(pipes[i][j][k]);
                }
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
            perror("Unsuccesful wait");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}