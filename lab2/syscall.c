#include "syscall.h"
#include "console_buf.h"
#include "kos.h"
#include "simulator.h"
#include "scheduler.h"
#include "kt.h"
#include "errno.h"

void syscall_return(struct PCB* pcb, int return_value) {
    pcb->registers[PCReg] = pcb->registers[NextPCReg];
    pcb->registers[2] = return_value;
    dll_append(readyQueue, new_jval_v((void *)pcb));
    kt_exit();
}

bool ValidAddress(struct PCB* pcb) {
    struct PCB *pcb = (struct PCB*)arg;
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

    struct KOSstat *kosstat = (struct KOSstat *)&main_memory[pcb->registers[6]+ pcb->mem_base];
    if (arg1 == 0) {
        stat_buf_fill(kosstat, 1);
    }
    if (arg1 == 2 || arg1 == 1) {
        stat_buf_fill(kosstat, 256);
    }
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