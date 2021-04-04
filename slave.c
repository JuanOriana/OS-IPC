// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include "consts.h"

void solve(char *file);

int main(int argc, char const *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    char *path = NULL;
    size_t len = 0;

    while ((getline(&path, &len, stdin)) > 0)
    {
        path[strcspn(path, "\n")] = 0; // Removing /n
        solve(path);
    }

    free(path);
    return 0;
}

void solve(char *file)
{
    char command[BUFF_SIZE];
    char buff[MAX_OUTPUT_SIZE];
    int retValue;

    retValue = sprintf(command, "minisat %s | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\" | tr \"\\n\" \"\t\" | tr -d \" \t\"", file);
    if (retValue < 0)
    {
        perror("sprintf");
        exit(EXIT_FAILURE);
    }

    FILE *stream = popen(command, "r");
    if (stream == NULL)
    {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    if (fgets(buff, MAX_OUTPUT_SIZE, stream) == NULL)
    {
        perror("fgets");
        exit(EXIT_FAILURE);
    }

    if (pclose(stream) < 0)
    {
        perror("pclose");
        exit(EXIT_FAILURE);
    }

    int variables, clauses;
    float cpuTime;
    char state[14];

    if (sscanf(buff, "Numberofvariables:%10dNumberofclauses:%10dCPUtime:%10fs%13s", &variables, &clauses, &cpuTime, state) == EOF)
    {
        perror("sscanf");
        exit(EXIT_FAILURE);
    }

    printf("PID:%d Filename:%s Numberofvariables:%d Numberofclauses:%d CPUtime:%f %s\n", getpid(), basename(file), variables,
           clauses, cpuTime, state);
}
