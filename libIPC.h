#ifndef LIB_IPC_
#define LIB_IPC_

#define PIPES_PER_CHILD 2
#define FILEDESC_QTY 2
#define READ_END 0
#define WRITE_END 1
#define SLAVE_TO_MASTER 0
#define MASTER_TO_SLAVE 1

#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "errorHandling.h"



int initPipes(int pipeMat[][PIPES_PER_CHILD][FILEDESC_QTY], int pipeCount, int *maxFd);
int initForks(int *childIDs, int childCount, int pipes[][PIPES_PER_CHILD][FILEDESC_QTY]);
int closePipes(int pipeCount, int pipes[][PIPES_PER_CHILD][FILEDESC_QTY]);
void buildReadSet(fd_set *set, int pipes[][2][2], char closedPipes[][2], int childCount);
int waitAll(int *childIDs, int childCount);




#endif