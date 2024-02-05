/*
 * kos.c -- starting point for student's os.
 *
 */
#include <stdlib.h>

#include "simulator.h"
#include "jval.h"
#include "dllist.h"
#include "scheduler.h"
#include "kt.h"
#include "syscall.h"

Dllist readyQueue;
struct PCB *running;

int readBuffer[256];
int readHead = 0;
int readTail = 0;

kt_sem write_ok;
kt_sem writers;
kt_sem nelem;
kt_sem nslots;
kt_sem consoleWait;

void scheduler() {
    kt_joinall();
    struct PCB *p;
    if (running != NULL) {
        run_user_code(running->registers);
    } else if (dll_empty(readyQueue)) {
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
    // Initialize console write buffer
    write_ok = make_kt_sem(0);
    writers = make_kt_sem(1);

    // Initialize console read buffer
    consoleWait = make_kt_sem(0);
    nelem = make_kt_sem(0);
    nslots = make_kt_sem(ReadBufferSize);

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

void KOS() {
    readyQueue = new_dllist();
    kt_fork(initialize_user_process, kos_argv);
    kt_fork(console_reader_thread, NULL);
    kt_joinall();
    scheduler();
}