#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "dllist.h"
#include "simulator.h"
#include "kt.h"
#include "jrb.h"

struct PCB {
    int registers[NumTotalRegs];
    int sbrk;
    int mem_base;
    int mem_limit;

    unsigned short pid;
    int mem_bin;
    int return_value;
};

extern Dllist readyQueue;
extern struct PCB *running;

extern int currentPID;
extern JRB processTree;

int get_new_pid();
void destroy_pid(int pid);

void initialize_scheduler(void);
void *initialize_user_process(void *arg);
int perform_execve(struct PCB *job, char *fn, char **argv);
void scheduler(void);

#endif // SCHEDULER_H