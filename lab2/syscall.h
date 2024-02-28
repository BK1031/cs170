#ifndef SYSCALL_H
#define SYSCALL_H

#include "scheduler.h"

void syscall_return(struct PCB* pcb, int return_value);

bool ValidAddress(struct PCB* pcb);

void do_ioctl(struct PCB* pcb);
void do_fstat(struct PCB* pcb);
void getpagesize(struct PCB* pcb);
void do_sbrk(struct PCB* pcb);
void do_execve(struct PCB* pcb);
void execve_return(struct PCB *pcb, int return_value);
void do_write(struct PCB* pcb);
void do_read(struct PCB* pcb);


#endif // SYSCALL_H