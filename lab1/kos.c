/*
 * kos.c -- starting point for student's os.
 *
 */
#include <stdlib.h>

#include "simulator.h"

void KOS()
{
    int registers[NumTotalRegs];
    int i;

    bzero(main_memory, MemorySize);

    if (load_user_program("a.out") < 0) {
        fprintf(stderr,"Can't load program.\n");
        exit(1);
    }

    for (i=0; i < NumTotalRegs; i++)
        registers[i] = 0;

    /* set up the program counters and the stack register */

    registers[PCReg] = 0;
    registers[NextPCReg] = 4;

    /* need to back off from top of memory */
    /* 12 for argc, argv, envp */
    /* 12 for stack frame */
    registers[StackReg] = MemorySize - 24;

    printf("Running user code.\n");

    run_user_code(registers);

    /* not reached .. */
}