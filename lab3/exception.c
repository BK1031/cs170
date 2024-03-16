/*
 * exception.c -- stub to handle user mode exceptions, including system calls
 *
 * Everything else core dumps.
 *
 * Copyright (c) 1992 The Regents of the University of California. All rights
 * reserved.  See copyright.h for copyright notice and limitation of
 * liability and disclaimer of warranty provisions.
 */
#include <stdlib.h>

#include "dllist.h"
#include "simulator.h"
#include "scheduler.h"
#include "kos.h"
#include "kt.h"
#include "syscall.h"
#include "console_buf.h"

void exceptionHandler(ExceptionType which)
{
    int type, r5, r6, r7, newPC;
    //int buf[NumTotalRegs];

    if (!is_noop) {
        examine_registers(running->registers);
    }
    type = running->registers[4];
    r5 = running->registers[5];
    r6 = running->registers[6];
    r7 = running->registers[7];
    newPC = running->registers[NextPCReg];

    /*
     * for system calls type is in r4, arg1 is in r5, arg2 is in r6, and
     * arg3 is in r7 put result in r2 and don't forget to increment the
     * pc before returning!
     */

    switch (which)
    {
        case SyscallException:
            /* the numbers for system calls is in <sys/syscall.h> */
            switch (type)
            {
                case 0:
                    /* 0 is our halt system call number */
                    DEBUG('e', "Halt initiated by user program\n");
                    SYSHalt();
                case SYS_exit:
                    /* this is the _exit() system call */
                    kt_fork((void *)do_exit, (void *) running);
                    DEBUG('e', "exit() system call\n");
                    break;
                case SYS_write:
                    kt_fork((void *) do_write, (void *) running);
                    break;
                case SYS_read:
                    kt_fork((void *) do_read, (void *) running);
                    break;
                case SYS_ioctl:
                    kt_fork((void*) ioctl, (void *) running);
                    break;
                case SYS_fstat:
                    kt_fork((void*) fstat, (void *) running);
                    break;
                case SYS_getpagesize:
                    kt_fork((void*) getpagesize, (void *) running);
                    break;
                case SYS_sbrk:
                    kt_fork((void*) do_sbrk, (void *) running);
                    break;
                case SYS_execve:
                    kt_fork((void *) do_execve, (void *) running);
                    break;
                case SYS_getpid:
                    kt_fork((void *) getpid, (void *) running);
                    break;
                case SYS_fork:
                    kt_fork((void *) do_fork, (void *) running);
                    break;
                case SYS_getdtablesize:
                    kt_fork((void*) getdtablesize, (void *) running);
                    break;
                case SYS_close:
                    kt_fork((void*) do_close, (void *) running);
                    break;
                case SYS_wait:
                    kt_fork((void*) do_wait, (void *) running);
                    break;
                case SYS_getppid:
                    kt_fork((void *) get_ppid, (void *) running);
                    break;
                default:
                    DEBUG('e', "Unknown system call\n");
                    SYSHalt();
                    break;
            }
            break;
        case PageFaultException:
            DEBUG('e', "Exception PageFaultException\n");
            break;
        case BusErrorException:
            DEBUG('e', "Exception BusErrorException\n");
            break;
        case AddressErrorException:
            DEBUG('e', "Exception AddressErrorException\n");
            break;
        case OverflowException:
            DEBUG('e', "Exception OverflowException\n");
            break;
        case IllegalInstrException:
            DEBUG('e', "Exception IllegalInstrException\n");
            break;
        default:
            printf("Unexpected user mode exception %d %d\n", which, type);
            exit(1);
    }
    scheduler();
}

void interruptHandler(IntType which)
{
    if (!is_noop) {
        running->registers[NumTotalRegs];
        examine_registers(running->registers);
        dll_append(readyQueue, new_jval_v((void *) running));
    }

    switch (which) {
        case ConsoleReadInt:
            DEBUG('e', "ConsoleReadInt interrupt\n");
            V_kt_sem(consoleWait);
            break;
        case ConsoleWriteInt:
            DEBUG('e', "ConsoleWriteInt interrupt\n");
            V_kt_sem(writeok);
            break;
        case TimerInt:
            DEBUG('e', "TimerInt interrupt\n");
            break;
        default:
            DEBUG('e', "Unknown interrupt\n");
            break;
    }
    scheduler();
}