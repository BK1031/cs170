#include <stdlib.h>
#include "syscall.h"
#include "console_buf.h"
#include "kos.h"
#include "simulator.h"
#include "scheduler.h"
#include "kt.h"
#include "errno.h"
#include "memory.h"

void syscall_return(struct PCB* pcb, int return_value) {
    pcb->registers[PCReg] = pcb->registers[NextPCReg];
    pcb->registers[2] = return_value;
    dll_append(readyQueue, new_jval_v((void *)pcb));
    kt_exit();
}

bool ValidAddress(struct PCB* pcb) {
    if (pcb->registers[RetAddrReg] >= 0 && pcb->registers[RetAddrReg] <= pcb->mem_limit) {
        return TRUE;
    }
    return FALSE;
}

void do_ioctl(struct PCB* pcb) {
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    if (!ValidAddress(pcb)) {
        syscall_return(pcb, -EFAULT);
    }

    if (arg1 != 1 || arg2 != JOS_TCGETP) {
        syscall_return(pcb, -EINVAL);
    }

    struct JOStermios *input= (struct JOStermios *)&main_memory[arg3];
    ioctl_console_fill(input);
    syscall_return(pcb,0);
}

void do_fstat(struct PCB* pcb) {
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    if (!ValidAddress(pcb)) {
        syscall_return(pcb, -EFAULT);
    }

    struct KOSstat *kosstat = (struct KOSstat *)&main_memory[arg2 + pcb->mem_base];
    if (arg1 == 0) {
        stat_buf_fill(kosstat, 1);
    }
    if (arg1 == 2 || arg1 == 1) {
        stat_buf_fill(kosstat, 256);
    }
    syscall_return(pcb, 0);
}

void getpagesize(struct PCB* pcb) {
    syscall_return(pcb, PageSize);
}

void do_sbrk(struct PCB* pcb) {
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    if (!ValidAddress(pcb)) {
        syscall_return(pcb, -EFAULT);
    }

    if (pcb->sbrk == NULL) {
        pcb->sbrk = 0;
    }

    int old_sbrk = pcb->sbrk;
    int new_sbrk = old_sbrk + arg1;
    if (new_sbrk > User_Limit) {
        syscall_return(pcb, -ENOMEM);
    }
    pcb->sbrk = new_sbrk;
    syscall_return(pcb, old_sbrk);
}

void do_execve(struct PCB* pcb) {
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    char **my_argv = (char **)(arg2 + main_memory + pcb->mem_base);
    int NumArgs = 1;
    for (int i = 0; my_argv[i] != NULL; i++) {
        NumArgs++;
    }

    char **array = malloc(NumArgs * sizeof(char *));
    char *filename = strdup(arg1 + main_memory + pcb->mem_base);

    int *offset;
    char *argArray;
    int j = 0;
    for (int i = 0; i < NumArgs - 1; i++){
        offset = (int *)(arg2 + main_memory + pcb->mem_base + j);
        argArray = (char *)(*offset + main_memory + pcb->mem_base);
        array[i] = strdup(argArray);
        j += 4;
    }
    array[NumArgs - 1] = '\0';
    int result = perform_execve(pcb, filename, array);
    if (result == 0) {
        execve_return(pcb, 0);
    }
    free(filename);
    free(array);
    execve_return(pcb, -EINVAL);
}

void execve_return(struct PCB *pcb, int return_value) {
    pcb->registers[PCReg] = 0;
    pcb->registers[NextPCReg] =  4;
    pcb->registers[2] = return_value;
    dll_append(readyQueue, new_jval_v((void *)pcb));
    kt_exit();
}

void getpid(struct PCB *pcb) {
    syscall_return(pcb, pcb->pid);
}

void do_fork(struct PCB* pcb){
    struct PCB *newProc = (struct PCB*)malloc(sizeof(struct PCB));
    bool hasSpace = FALSE;
    for (int i = 0; i < 8; i++) {
        if (memory_partitions[i] == 0) {
            memory_partitions[i] = 1;
            newProc->mem_base = i * User_Limit;
            newProc->mem_limit = User_Limit;
            memcpy(main_memory + newProc->mem_base, main_memory + pcb->mem_base, User_Limit);

            for (int j = 0; j < NumTotalRegs; j++) {
                newProc->registers[j] = pcb->registers[j];
            }
            newProc->sbrk = pcb->sbrk;
            newProc->pid = get_new_pid();

            hasSpace = TRUE;
            break;
        }
    }

    if (hasSpace) {
        kt_fork((void *) finish_fork, newProc);
        syscall_return(pcb, newProc->pid);
    } else {
        syscall_return(pcb, -EAGAIN);
    }
}

void finish_fork(struct PCB* pcb) {
    syscall_return(pcb, 0);
}


void do_write(struct PCB* pcb) {
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    if (arg1 != 1 && arg1 != 2) {
        syscall_return(pcb, -EBADF);
    } else if (arg2 < 0) {
        syscall_return(pcb, -EFAULT);
    } else if (arg3 < 0) {
        syscall_return(pcb, -EINVAL);
    } else if (arg2 + arg3 > MemorySize - 12) {
        syscall_return(pcb, -EFBIG);
    }

    P_kt_sem(writers);
    for (int i = 0; i < arg3; i++) {
        char c = main_memory[arg2+i];
        console_write(c);
        P_kt_sem(write_ok);
    }
    V_kt_sem(writers);
    syscall_return(pcb, arg3);
}

void do_read(struct PCB* pcb) {
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    if (arg1 != 0) {
        syscall_return(pcb, -EBADF);
    } else if (arg2 < 0) {
        syscall_return(pcb, -EFAULT);
    } else if (arg3 < 0) {
        syscall_return(pcb, -EINVAL);
    } else if (arg2 + arg3 > MemorySize - 12) {
        syscall_return(pcb, -EFBIG);
    }

    for (int i = 0; i < arg3; i++) {
        P_kt_sem(nelem);

        char c = readBuffer[readHead];
        readHead = (readHead + 1) % ReadBufferSize;

        V_kt_sem(nslots);

        if (c < 0) {
            syscall_return(pcb, i);
        }

        main_memory[arg2+i] = c;
    }

    syscall_return(pcb, arg3);
}