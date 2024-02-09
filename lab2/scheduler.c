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
        run_user_code(p->registers);
    }
}

void *initialize_user_process(void *arg) {
    char **my_argv = (char **)arg;

    struct PCB *p = (struct PCB *)malloc(sizeof(struct PCB));

    for (int i=0; i < NumTotalRegs; i++) {
        p->registers[i] = 0;
    }

    load_user_program(my_argv[0]);
    p->registers[PCReg] = 0;
    p->registers[NextPCReg] = 4;
    p->registers[StackReg] = MemorySize - 12;

    int *user_args = MoveArgsToStack(p->registers,my_argv,0);
    InitCRuntime(user_args,p->registers,my_argv,0);

    dll_append(readyQueue, new_jval_v((void *)p));
    kt_exit();
}