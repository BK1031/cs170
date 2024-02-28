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

    struct PCB* my_pcb = (struct PCB*)malloc(sizeof(struct PCB));
    for (int i=0; i < NumTotalRegs; i++) {
        my_pcb->registers[i] = 0;
    }

    my_pcb->registers[PCReg] = 0;
    my_pcb->registers[NextPCReg] = 4;
    my_pcb->mem_base = User_Base;
    my_pcb->mem_limit = User_Limit;
    memory_partitions[0] = 1;
    my_pcb->mem_slot = 0;

    my_pcb->pid = get_new_pid();
    my_pcb->parent = init;
    my_pcb->waiters_sem = make_kt_sem(0);
    my_pcb->waiters = new_dllist();
    my_pcb->children = make_jrb();

    jrb_insert_int(init->children, my_pcb->pid, new_jval_v((void *)my_pcb));

    char **my_argv = (char **)arg;
    int result = perform_execve(my_pcb, my_argv[0], my_argv);
    if (result == 0) {
        dll_append(readyQueue, new_jval_v((void*)my_pcb));
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