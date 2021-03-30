#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#define BUFF_SIZE 1024
#define MAX_FILE_LENGTH 4096
#define NEGATIVE_ERROR(error, function) if(error == -1) {perror(function);  exit(EXIT_FAILURE);}
#define NULL_ERROR(error, function) if(error == NULL) {perror(function);  exit(EXIT_FAILURE);}
#define EOF_ERROR(error, function) if(error == EOF) {perror(function);  exit(EXIT_FAILURE);}

void solve(char *file);

int main(int argc, char const *argv[])
{
    setvbuf(stdout,NULL,_IONBF,0);
    char *path = NULL;
    size_t len = 0;
    ssize_t lineSize = 0;

    while ((lineSize = getline(&path, &len, stdin)) > 0)
    {
        if(strcmp(path,"") == 0)
            return 0;

        path[strcspn(path, "\n")] = 0; //Clearing path ; path = nombre del archivo

        solve(path);

    }
    free(path);
    return 0;
}


void solve(char *file) {
    char command[MAX_FILE_LENGTH] = "";
    char buff[MAX_FILE_LENGTH] = "";
    int retValue = -1;

    retValue = sprintf(command,"minisat %s | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\" | tr \"\\n\" \"\t\" | tr -d \" \t\"", file);
    NEGATIVE_ERROR(retValue, "sprintf");

    FILE *stream = popen(command, "r");
    NULL_ERROR(stream, "popen");

    char *p = fgets(buff, MAX_FILE_LENGTH, stream);
    NULL_ERROR(p, "fgets");

    retValue = pclose(stream);
    NEGATIVE_ERROR(retValue, "pclose");

    int variables, clauses = -1;
    float cpuTime = -1;
    char state[14] = "";

    retValue = sscanf(buff, "Numberofvariables:%dNumberofclauses:%dCPUtime:%fs%s", &variables, &clauses, &cpuTime, state);
    EOF_ERROR(retValue, "sscanf");

    printf("PID:%d\nFilename:%s\nNumberofvariables:%d Numberofclauses:%d CPUtime:%f %s\n", getpid(), basename(file), variables,
                clauses, cpuTime, state);

}
