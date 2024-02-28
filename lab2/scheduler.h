#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "dllist.h"
#include "simulator.h"
#include "kt.h"

struct PCB {
    int registers[NumTotalRegs];
    int sbrk;
    int mem_base;
    int mem_limit;

    unsigned short pid;
};

extern Dllist readyQueue;
extern struct PCB *running;

void initialize_scheduler(void);
void *initialize_user_process(void *arg);
int perform_execve(struct PCB *job, char *fn, char **argv);
void scheduler(void);

#endif // SCHEDULER_H