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

void
exceptionHandler(ExceptionType which)
{
    int             type, r5, r6, r7, newPC;
    int             buf[NumTotalRegs];

    examine_registers(buf);
    type = buf[4];
    r5 = buf[5];
    r6 = buf[6];
    r7 = buf[7];
    newPC = buf[NextPCReg];

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
                    printf("Program exited with value %d.\n", r5);
                    SYSHalt();
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
    noop();
}

void
interruptHandler(IntType which)
{
    switch (which) {
        case ConsoleReadInt:
            DEBUG('e', "ConsoleReadInt interrupt\n");
            noop();
            break;
        case ConsoleWriteInt:
            DEBUG('e', "ConsoleWriteInt interrupt\n");
            noop();
            break;
        default:
            DEBUG('e', "Unknown interrupt\n");
            noop();
            break;
    }
}