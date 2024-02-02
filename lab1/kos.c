/*
 * kos.c -- starting point for student's os.
 *
 */
#include <stdlib.h>

#include "simulator.h"
#include "jval.h"
#include "dllist.h"

struct PCB {
    int registers[NumTotalRegs];
};

Dllist readyQueue;
struct PCB *running;

void scheduler()
{
    readyQueue = new_dllist();
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

void KOS()
{
    scheduler();
}