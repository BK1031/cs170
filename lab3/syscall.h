#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdlib.h>

#include "console_buf.h"
#include "simulator.h"
#include "scheduler.h"
#include "kt.h"
#include "dllist.h"
#include <errno.h>

extern void syscall_return(struct PCB* pcb, int value);

extern bool ValidAddress(struct PCB* pcb);
extern void ioctl(struct PCB* pcb);
extern void fstat(struct PCB* pcb);
extern void do_sbrk(struct PCB* pcb);

extern void do_execve(struct PCB* pcb);
extern void execve_return(struct PCB* pcb, int value);

extern void do_fork(struct PCB* pcb);
void finish_fork(struct PCB* pcb);

extern void do_exit(struct PCB* pcb);
extern void do_close(struct PCB* pcb);
extern void do_wait(struct PCB* pcb);

extern void getpagesize(struct PCB* pcb);
extern void getdtablesize(struct PCB* pcb);
extern void getpid(struct PCB* pcb);
extern void get_ppid(struct PCB* pcb);

extern void do_dup(struct PCB* pcb);
extern void do_dup2(struct PCB* pcb);
extern void do_pipe(struct PCB* pcb);

extern void *do_write(struct PCB* pcb);
extern void *do_read(struct PCB* pcb);

#endif