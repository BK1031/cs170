#include <stdlib.h>
#include <stdio.h>

#include "jrb.h"
#include "dllist.h"
#include "simulator.h"
#include "scheduler.h"
#include "kt.h"
#include "errno.h"

Dllist readyQueue;
struct PCB *running;

int currentPID;
JRB processTree;
bool is_noop;
struct PCB *init;

void initialize_scheduler() {
    readyQueue = new_dllist();
    processTree = make_jrb();
    initialize_memory();
    is_noop = TRUE;
}

void scheduler() {
    kt_joinall();
    if (dll_empty(readyQueue)) {
        running = NULL;
        if (jrb_empty(init->children)) {
            SYSHalt();
        }
        is_noop = TRUE;
        noop();
    } else {
        struct PCB *p = (struct PCB *)(jval_v(dll_val(dll_first(readyQueue))));
        running = p;
        dll_delete_node(dll_first(readyQueue));
        start_timer(10);
        is_noop = FALSE;
        User_Base = running->mem_base;
        User_Limit = running->mem_limit;

        run_user_code(running->registers);
    }
}

void* initialize_user_process(void* arg) {
    User_Base = 0;
    bzero(main_memory, MemorySize);

    init = (struct PCB*)malloc(sizeof(struct PCB));
    for (int i=0; i < NumTotalRegs; i++) {
        init->registers[i] = 0;
    }

    init->pid = get_new_pid();
    init->waiters_sem = make_kt_sem(0);
    init->waiters = new_dllist();
    init->children = make_jrb();

    struct PCB* p = (struct PCB*)malloc(sizeof(struct PCB));
    for (int i=0; i < NumTotalRegs; i++) {
        p->registers[i] = 0;
    }

    p->registers[PCReg] = 0;
    p->registers[NextPCReg] = 4;
    p->mem_base = User_Base;
    p->mem_limit = User_Limit;
    memory_partitions[0] = 1;
    p->mem_slot = 0;

    p->pid = get_new_pid();
    p->parent = init;
    p->waiters_sem = make_kt_sem(0);
    p->waiters = new_dllist();
    p->children = make_jrb();

    jrb_insert_int(init->children, p->pid, new_jval_v((void *)p));

    for (int i = 0; i < 64; i++) {
        p->fd[i] = (struct Fd*)malloc(sizeof(struct FD));
    }

    // stdin
    p->fd[0]->console = TRUE;
    p->fd[0]->open = TRUE;
    p->fd[0]->is_read = TRUE;
    p->fd[0]->reference_count = 0;
    p->fd[0]->pipe = NULL;

    // stdout
    p->fd[1]->console = TRUE;
    p->fd[1]->open = TRUE;
    p->fd[1]->is_read = FALSE;
    p->fd[1]->reference_count = 0;
    p->fd[1]->pipe = NULL;

    // stderr
    p->fd[2]->console = TRUE;
    p->fd[2]->open = TRUE;
    p->fd[2]->is_read = FALSE;
    p->fd[2]->reference_count = 0;
    p->fd[2]->pipe = NULL;

    for (int i = 3; i < 64; i++) {
        p->fd[i]->console = FALSE;
        p->fd[i]->open = FALSE;
        p->fd[i]->is_read = FALSE;
        p->fd[i]->reference_count = 0;
        p->fd[i]->pipe = NULL;
    }

    char **my_argv = (char **)arg;
    int result = perform_execve(p, my_argv[0], my_argv);
    if (result == 0) {
        dll_append(readyQueue, new_jval_v((void*)p));
    }

    kt_exit();
}

int perform_execve(struct PCB* pcb, char* filename, char** pcb_argv) {
    if (load_user_program(filename) < 0) {
        fprintf(stderr,"Can't load program.\n");
        return -EFBIG;
    }

    pcb->registers[PCReg] = 0;
    pcb->registers[NextPCReg] = 4;
    pcb->registers[StackReg] = pcb->mem_limit - 12;
    pcb->sbrk_ptr = (void *)load_user_program(filename);
    int *user_args = MoveArgsToStack(pcb->registers,pcb_argv,pcb->mem_base);
    InitCRuntime(user_args,pcb->registers,pcb_argv,pcb->mem_base);

    return 0;
}

int get_new_pid() {
    currentPID = 0;
    while (jrb_find_int(processTree, currentPID)!= NULL) {
        currentPID++;
    }
    jrb_insert_int(processTree, currentPID, new_jval_i(0));
    return currentPID;
}

void destroy_pid(int pid) {
    JRB node = jrb_find_int(processTree, pid);
    if (!node) {
        return;
    }
    jrb_delete_node(node);
}