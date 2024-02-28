#include <stdlib.h>

#include "simulator.h"
#include "jval.h"
#include "dllist.h"
#include "scheduler.h"
#include "kt.h"

Dllist readyQueue;
struct PCB *running;

void initialize_scheduler() {
    readyQueue = new_dllist();
    running = NULL;
}

void scheduler() {
    kt_joinall();
    struct PCB *p;
    if (dll_empty(readyQueue)) {
        running = NULL;
        noop();
    } else {
        p = (struct PCB *) dll_first(readyQueue)->val.v;
        dll_delete_node(dll_first(readyQueue));
        running = p;
        User_Base = p->mem_base;
        User_Limit = p->mem_limit;
        run_user_code(p->registers);
    }
}

void *initialize_user_process(void *arg) {
    User_Base = 0;
    User_Limit = MemorySize;

    char **my_argv = (char **)arg;

    struct PCB *p = (struct PCB *)malloc(sizeof(struct PCB));

    User_Base = 0;
    User_Limit = MemorySize;

    for (int i=0; i < NumTotalRegs; i++) {
        p->registers[i] = 0;
    }

    load_user_program(my_argv[0]);
    p->registers[PCReg] = 0;
    p->registers[NextPCReg] = 4;
    p->registers[StackReg] = MemorySize - 12;

    p->mem_base = User_Base;
    p->mem_limit = User_Limit;

    perform_execve(p, my_argv[0], my_argv);
    dll_append(readyQueue, new_jval_v((void *)p));

//    int *user_args = MoveArgsToStack(p->registers,my_argv,0);
//    InitCRuntime(user_args,p->registers,my_argv,0);

    kt_exit();
}

int perform_execve(struct PCB *job, char *fn, char **argv) {
    if (load_user_program(fn) < 0) {
        fprintf(stderr,"Can't load program.\n");
        exit(1);
    }

    job->registers[PCReg] = 0;
    job->registers[NextPCReg] = 4;
    job->registers[StackReg] = job->mem_limit - 12;
    job->sbrk = load_user_program(fn);
    int *user_args = MoveArgsToStack(job->registers,argv,job->mem_base);
    InitCRuntime(user_args,job->registers,argv,job->mem_base);

    return 0;
}