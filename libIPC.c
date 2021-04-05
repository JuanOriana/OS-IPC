#include "libIPC.h"


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

