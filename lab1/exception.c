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

#include "simulator.h"
#include "scheduler.h"
#include "kt.h"
#include "syscall.h"

void exceptionHandler(ExceptionType which) {
    int             type, r5, r6, r7, newPC;
    int             buf[NumTotalRegs];

    examine_registers(running->registers);
    type = running->registers[4];
    r5 = running->registers[5];
    r6 = running->registers[6];
    r7 = running->registers[7];
    newPC = running->registers[NextPCReg];

//    printf("Exception %d, type %d, r5 %d, r6 %d, r7 %d, newPC %d\n", which, type, r5, r6, r7, newPC);

    /*
     * for system calls type is in r4, arg1 is in r5, arg2 is in r6, and
     * arg3 is in r7 put result in r2 and don't forget to increment the
     * pc before returning!
     */

    switch (which) {
        case SyscallException:
            /* the numbers for system calls is in <sys/syscall.h> */
            switch (type) {
                case 0:
                    /* 0 is our halt system call number */
                    DEBUG('e', "Halt initiated by user program\n");
                    SYSHalt();
                case SYS_exit:
                    /* this is the _exit() system call */
                    DEBUG('e', "_exit() system call\n");
                    for (int i = 0; i < ReadBufferSize; i++) {
                        printf("%c", readBuffer[i]);
                    }
                    printf("Program exited with value %d.\n", r5);
                    SYSHalt();
                case SYS_write:
                    kt_fork(do_write, (void*) running);
                    break;
                case SYS_read:
                    kt_fork(do_read, (void*) running);
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

void interruptHandler(IntType which) {
    switch (which) {
        case ConsoleReadInt:
            DEBUG('e', "ConsoleReadInt interrupt\n");
            V_kt_sem(consoleWait);
            scheduler();
            break;
        case ConsoleWriteInt:
            DEBUG('e', "ConsoleWriteInt interrupt\n");
//            V_kt_sem(write_ok);
            scheduler();
            break;
        default:
            DEBUG('e', "Unknown interrupt\n");
            scheduler();
            break;
    }
}