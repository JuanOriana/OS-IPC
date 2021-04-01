#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

#define SHMEM_PATH "/shmemBuffer"

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
        scanf("%d", fileCount);
    }

    int shmFd = shm_open(SHMEM_PATH, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR);
    char *shmBase = mmap(NULL, MAX_OUTPUT_SIZE * fileCount + sizeof(long), PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);

    close(smhFd);
    if (shm_unlink(SHMEM_PATH) < 0)
    {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }
    return 0;
}
