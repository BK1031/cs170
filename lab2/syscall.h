#ifndef SYSCALL_H
#define SYSCALL_H

#include "scheduler.h"

void syscall_return(struct PCB* pcb, int return_value);

void do_write(struct PCB* pcb);
void do_read(struct PCB* pcb);


#endif // SYSCALL_H