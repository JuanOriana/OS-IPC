#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define CHILD_COUNT 3
#define READ_END 0
#define WRITE_END 1
#define SLAVE_TO_MASTER 0
#define MASTER_TO_SLAVE 1

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <files>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fileCount = argc - 1;
    int currIdx = 1;
    pid_t childIDs[CHILD_COUNT];
    // 2 pipes per child
    int pipes[CHILD_COUNT][2][2];
    char *const *execParam = NULL;

    //Pipe inits
    for (int i = 0; i < CHILD_COUNT; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            if (pipe(pipes[i][j]) < 0)
            {
                perror("Unsuccesful pipe");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Fork inits
    for (int i = 0; i < CHILD_COUNT; i++)
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

    // Wait to end
    for (int i = 0; i < CHILD_COUNT; i++)
    {
        if (waitpid(childIDs[i], NULL, 0) < 0)
        {
            perror("Unsuccesful wait");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
