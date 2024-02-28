#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "jrb.h"
#include "memory.h"
#include "kt.h"
#include "kos.h"
#include "dllist.h"

struct PCB {
    int registers[NumTotalRegs];
    void* sbrk_ptr;

    int mem_limit;
    int mem_base;

    unsigned short pid;
    int return_value;
    int mem_slot;

    struct PCB* parent;

    kt_sem waiters_sem;
    Dllist waiters;
    JRB children;
};

extern Dllist readyQueue;
extern struct PCB *running;

extern int currentPID;
extern JRB processTree;

extern bool is_noop;
extern struct PCB* init;

void initialize_scheduler();

void* initialize_user_process(void* arg);

void scheduler();

int perform_execve(struct PCB *pcb, char *filename, char **pcb_argv);

int get_new_pid();
void destroy_pid(int pid);

#endif