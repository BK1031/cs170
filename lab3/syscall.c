#include <stdlib.h>

#include "console_buf.h"
#include "simulator.h"
#include "scheduler.h"
#include "jrb.h"
#include "kt.h"
#include "dllist.h"
#include "syscall.h"

void syscall_return(struct PCB *pcb, int value) {
    pcb->registers[PCReg] = pcb->registers[NextPCReg];
    pcb->registers[2] = value;
    dll_append(readyQueue, new_jval_v((void *)pcb));
    kt_exit();
}

bool ValidAddress(struct PCB* pcb){
    if (pcb->registers[RetAddrReg] >= 0 && pcb->registers[RetAddrReg] <= pcb->mem_limit) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void ioctl(struct PCB* pcb){
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    if (!ValidAddress(pcb)) {
        syscall_return(pcb, -EFAULT);
    }
    if (arg1 != 1 || arg2 != JOS_TCGETP) {
        syscall_return(pcb,-EINVAL);
    }

    struct JOStermios *input= (struct JOStermios *)&main_memory[arg3 + pcb->mem_base];
    ioctl_console_fill(input);
    syscall_return(pcb,0);
}

void fstat(struct PCB* pcb) {
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    if (!ValidAddress(pcb)) {
        syscall_return(pcb, -EFAULT);
    }

    struct KOSstat *kosstat = (struct KOSstat *)&main_memory[arg2+ pcb->mem_base];
    if (arg1 == 0) {
        stat_buf_fill(kosstat, 1);
    }
    if (arg1 == 2 || arg1 == 1) {
        stat_buf_fill(kosstat, 256);
    }
    syscall_return(pcb, 0);
}

void do_sbrk(struct PCB* pcb) {
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    if (!ValidAddress(pcb)) {
        syscall_return(pcb, -EFAULT);
    }

    if (pcb->sbrk_ptr == NULL) {
        pcb->sbrk_ptr == 0;
    }

    int old = (int)pcb->sbrk_ptr;
    if (old + arg1 > User_Limit) {
        syscall_return(pcb, -ENOMEM);
    }
    pcb->sbrk_ptr += arg1;
    syscall_return(pcb, old);
}

void do_execve(struct PCB *pcb) {
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    char **my_argv = (char **)(arg2 + main_memory + pcb->mem_base);

    int numArg = 1;
    while (my_argv[numArg] != NULL) {
        numArg += 1;
    }

    char **array = malloc(numArg * sizeof(char *));
    char *filename = strdup(arg1 + main_memory + pcb->mem_base);

    int *offset;
    char *argArray;
    int j = 0;
    for (int i = 0; i < numArg; i++) {
        offset = (int *)(arg2 + main_memory + pcb->mem_base + j);
        argArray = (char *)(*offset+ main_memory + pcb->mem_base);
        array[i] = strdup(argArray);
        j += 4;
    }
    array[numArg] = '\0';

    int result = perform_execve(pcb, filename, array);

    free(filename);
    free(array);

    if (result == 0) {
        execve_return(pcb, 0);
    }
    execve_return(pcb, -EINVAL);
}

void execve_return(struct PCB *pcb, int value) {
    pcb->registers[PCReg] = 0;
    pcb->registers[NextPCReg] = 4;
    pcb->registers[2] = value;
    dll_append(readyQueue, new_jval_v((void *)pcb));
    kt_exit();
}

void do_fork(struct PCB* pcb) {
    struct PCB *newProc = (struct PCB*)malloc(sizeof(struct PCB));
    bool hasSpace = FALSE;
    for (int i = 0; i < 8; i++) {
        if (memory_partitions[i] == 0) {
            memory_partitions[i] = 1;
            newProc->mem_slot = i;
            newProc->mem_base = i * User_Limit;
            newProc->mem_limit = User_Limit;
            memcpy(main_memory + newProc->mem_base, main_memory + pcb->mem_base, User_Limit);

            for (int j = 0; j < NumTotalRegs; j++) {
                newProc->registers[j] = pcb->registers[j];
            }

            newProc->sbrk_ptr = pcb->sbrk_ptr;
            newProc->pid = get_new_pid();
            newProc->parent = pcb;
            newProc->waiters_sem = make_kt_sem(0);
            newProc->waiters = new_dllist();
            newProc->children = make_jrb();

            jrb_insert_int(pcb->children, newProc->pid, new_jval_v((void*) newProc));

            for (int k = 0; k < 64; k++) {
                newProc->fd[k] = (struct FD*)malloc(sizeof(struct FD));
                newProc->fd[k]->console = pcb->fd[k]->console;
                newProc->fd[k]->is_read = pcb->fd[k]->is_read;
                newProc->fd[k]->pipe = pcb->fd[k]->pipe;
                newProc->fd[k]->open = pcb->fd[k]->open;
                newProc->fd[k]->reference_count = pcb->fd[k]->reference_count;

                if (pcb->fd[k]->open == TRUE) {
                    if (pcb->fd[k]->console == FALSE) {
                        if (pcb->fd[k]->is_read == TRUE) {
                            pcb->fd[k]->pipe->read_count += 1;
                        } else {
                            pcb->fd[k]->pipe->write_count += 1;
                        }
                    }
                }
            }

            hasSpace = TRUE;
            break;
        }
    }
    if (hasSpace) {
        kt_fork((void *)finish_fork, newProc);
        syscall_return(pcb, newProc->pid);
    } else {
        syscall_return(pcb, -EAGAIN);
    }
}

void finish_fork(struct PCB* pcb) {
    syscall_return(pcb, 0);
}

void do_exit(struct PCB* pcb){
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    pcb->return_value = arg1;

    memory_partitions[pcb->mem_slot] = 0;
    memset(main_memory + pcb->mem_base, 0, pcb->mem_limit);

    jrb_delete_node(jrb_find_int(pcb->parent->children, pcb->pid));

    while (!jrb_empty(pcb->children)) {
        struct PCB *p = (struct PCB *)jval_v(jrb_val(jrb_first(pcb->children)));
        jrb_delete_node(jrb_find_int(pcb->children, p->pid));
        p->parent = init;
        jrb_insert_int(init->children, p->pid, new_jval_v((void *)p));
    }

    while (!dll_empty(pcb->waiters)) {
        struct PCB *wait_child = (struct PCB *)jval_v(dll_val(dll_first(pcb->waiters)));
        wait_child->parent = init;
        dll_append(init->waiters, dll_val(dll_first(pcb->waiters)));
        dll_delete_node(dll_first(pcb->waiters));
        V_kt_sem(init->waiters_sem);
    }

    for (int i = 0; i < 64; i++) {
        if (pcb->fd[i]->console == FALSE) {
            if (pcb->fd[i]->open == TRUE) {
                if (pcb->fd[i]->is_read == TRUE) {
                    pcb->fd[i]->pipe->read_count -= 1;
                } else {
                    pcb->fd[i]->pipe->write_count-= 1;
                }
                if (pcb->fd[i]->pipe->read_count == 0 && pcb->fd[i]->pipe->write_count == 0) {
                    free(pcb->fd[i]->pipe);
                }
                pcb->fd[i]->open = FALSE;
            }
        }
    }

    if (pcb->parent->pid == 0) {
        for (int i = 0; i < NumTotalRegs; i++) {
            pcb->registers[i] = 0;
        }
        destroy_pid(pcb->pid);
        jrb_free_tree(pcb->children);
        free_dllist(pcb->waiters);
        free(pcb);
    } else {
        V_kt_sem(pcb->parent->waiters_sem);
        dll_append(pcb->parent->waiters, new_jval_v((void*) pcb));
    }
    kt_exit();
}

void do_close(struct PCB* pcb) {
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    int fd_num = arg1;

    if (fd_num < 0 || fd_num >= 64) {
        syscall_return(pcb, -EBADF);
    }

    if (pcb->fd[fd_num]->open == FALSE) {
        syscall_return(pcb, -EBADF);
    }

    if (pcb->fd[fd_num]->console == FALSE) {
        if (pcb->fd[fd_num]->is_read == TRUE) {
            pcb->fd[fd_num]->pipe->read_count -= 1;
            pcb->fd[fd_num]->open = FALSE;
            if (pcb->fd[fd_num]->pipe->read_count == 0) {
                for (int f = 0; f < pcb->fd[fd_num]->pipe->write_count; f++) {
                    V_kt_sem(pcb->fd[fd_num]->pipe->space_available);
                }
                if (pcb->fd[fd_num]->pipe->write_count == 0) {
                    free(pcb->fd[fd_num]->pipe);
                }
            }
        } else {
            pcb->fd[fd_num]->pipe->write_count -= 1;
            pcb->fd[fd_num]->open = FALSE;
            if (pcb->fd[fd_num]->pipe->write_count==0) {
                if (pcb->fd[fd_num]->pipe->read_count==0) {
                    free(pcb->fd[fd_num]->pipe);
                }
            }
        }
    }

    syscall_return(pcb, 0);
}

void do_wait(struct PCB* pcb) {
    P_kt_sem(pcb->waiters_sem);

    struct PCB *completed_child = (struct PCB *)(jval_v((dll_val(dll_first(pcb->waiters)))));

    dll_delete_node(dll_first(pcb->waiters));
    destroy_pid(completed_child->pid);
    for (int i = 0; i < NumTotalRegs; i++) {
        completed_child->registers[i] = 0;
    }
    jrb_free_tree(completed_child->children);
    free_dllist(completed_child->waiters);

    int child_id = completed_child->pid;
    free(completed_child);

    syscall_return(pcb, child_id);
}

void getpagesize(struct PCB* pcb) {
    syscall_return(pcb, PageSize);
}

void getdtablesize(struct PCB* pcb) {
    syscall_return(pcb, 64);
}

void getpid(struct PCB* pcb) {
    syscall_return(pcb, pcb->pid);
}

void get_ppid(struct PCB* pcb) {
    syscall_return(pcb, pcb->parent->pid);
}

void do_dup(struct PCB* pcb) {
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    int old_fd = arg1;
    if (old_fd < 0 || old_fd >= 64) {
        syscall_return(pcb, -EBADF);
    }

    if (pcb->fd[old_fd]->open == FALSE) {
        syscall_return(pcb, -EBADF);
    }

    int new_fd = -1;
    int is_found = FALSE;
    for (int i = 0; i < 64; i++) {
        if (pcb->fd[i]->open == FALSE) {
            new_fd = i;
            is_found = TRUE;
            break;
        }
    }
    if (is_found == FALSE) {
        syscall_return(pcb, -EBADF);
    }

    pcb->fd[old_fd]->reference_count += 1;
    pcb->fd[new_fd] = pcb->fd[old_fd];

    if (pcb->fd[new_fd]->is_read == TRUE) {
        pcb->fd[new_fd]->pipe->read_count += 1;
    } else {
        pcb->fd[new_fd]->pipe->write_count += 1;
    }

    syscall_return(pcb, new_fd);
}

void do_dup2(struct PCB* pcb) {
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    int old_fd = arg1;
    if (old_fd < 0 || old_fd >= 64) {
        syscall_return(pcb, -EBADF);
    }

    if (pcb->fd[old_fd]->open == FALSE) {
        syscall_return(pcb, -EBADF);
    }

    int new_fd = arg2;
    if (new_fd < 0 || new_fd >= 64) {
        syscall_return(pcb, -EBADF);
    }

    if (pcb->fd[new_fd]->open == TRUE) {
        pcb->fd[new_fd]->console = FALSE;

        pcb->fd[new_fd]->pipe->read_count -= 1;
        pcb->fd[new_fd]->pipe->write_count -= 1;

        pcb->fd[new_fd]->pipe = NULL;
        pcb->fd[new_fd]->open = FALSE;
        pcb->fd[new_fd]->reference_count = 0;
    }

    pcb->fd[old_fd]->reference_count += 1;
    pcb->fd[new_fd] = pcb->fd[old_fd];

    if (pcb->fd[new_fd]->is_read == TRUE) {
        pcb->fd[new_fd]->pipe->read_count += 1;
    } else {
        pcb->fd[new_fd]->pipe->write_count += 1;
    }

    syscall_return(pcb, new_fd);
}

void do_pipe(struct PCB* pcb) {
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    struct Pipe *new_pipe = malloc(sizeof(struct Pipe));

    int read_Fd = -1;
    bool isFound1 = FALSE;
    for (int i = 0; i < 64; i++) {
        if (pcb->fd[i]->open == FALSE) {
            read_Fd = i;
            isFound1 = TRUE;
            break;
        }
    }
    if (isFound1 == FALSE) {
        syscall_return(pcb, -EMFILE);
    }
    pcb->fd[read_Fd]->open = TRUE;

    int write_Fd = -1;
    bool isFound2 = FALSE;
    for (int i = 0; i < 64; i++) {
        if (pcb->fd[i]->open == FALSE) {
            write_Fd = i;
            isFound2 = TRUE;
            break;
        }
    }
    if (isFound2 == FALSE) {
        syscall_return(pcb, -EMFILE);
    }
    pcb->fd[write_Fd]->open=TRUE;

    new_pipe->buffer = malloc(8192*sizeof(char));

    new_pipe->read_count = 1;
    new_pipe->write_count = 1;

    new_pipe->read = make_kt_sem(1);
    new_pipe->write = make_kt_sem(1);
    new_pipe->nelement = make_kt_sem(0);
    new_pipe->space_available = make_kt_sem(8192);

    new_pipe->write_head = 0;
    new_pipe->read_head = 0;

    new_pipe->writer_in_use = 0;

    pcb->fd[read_Fd]->pipe = new_pipe;
    pcb->fd[read_Fd]->is_read = TRUE;

    pcb->fd[write_Fd]->pipe = new_pipe;
    pcb->fd[write_Fd]->is_read = FALSE;

    memcpy(arg1 + main_memory + pcb->mem_base, &read_Fd, 4);
    memcpy(arg1 + main_memory + pcb->mem_base + 4, &write_Fd, 4);

    syscall_return(pcb, 0);
}

void *do_write(struct PCB* pcb) {
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    if(!ValidAddress(pcb)) {
        syscall_return(pcb, -EFAULT);
    }

    if (arg1 != 1 && arg1 != 2) {
        syscall_return(pcb, -EBADF);
    } else if (arg2 < 0) {
        syscall_return(pcb, -EFAULT);
    } else if (arg3 < 0) {
        syscall_return(pcb, -EINVAL);
    } else if (arg2 + arg3 > MemorySize) {
        syscall_return(pcb, -EFBIG);
    }

    P_kt_sem(writers);
    int local = (int)(arg2 + main_memory + pcb->mem_base);

    int write_count = 0;
    char *chars = (char *)(local);
    char *j = chars;


    for(int i = 0; i < arg3; i++){
        console_write(*j);
        P_kt_sem(writeok);
        j++;
        write_count++;
    }

    V_kt_sem(writers);
    syscall_return(pcb, write_count);
}

void *do_read(struct PCB* pcb) {
    int arg1 = pcb->registers[5];
    int arg2 = pcb->registers[6];
    int arg3 = pcb->registers[7];

    if(!ValidAddress(pcb)) {
        syscall_return(pcb, -EFAULT);
    }

    if (arg1 != 0) {
        syscall_return(pcb, -EBADF);
    } else if (arg2 < 0) {
        syscall_return(pcb, -EFAULT);
    } else if (arg3 < 0) {
        syscall_return(pcb, -EINVAL);
    } else if (arg2 + arg3 > MemorySize - 12) {
//        syscall_return(pcb, -EFBIG);
    }

    P_kt_sem(readers);
    int count = 0;
    for (int i = 0; i < arg3; i++){
        P_kt_sem(nelem);
        char c = (char)(consoleBuffer->buff[consoleBuffer->head]);

        if (c == -1){
            V_kt_sem(nslots);
            V_kt_sem(readers);
            syscall_return(pcb, count);
        }

        ((char *)(arg2 + main_memory + pcb->mem_base))[i] = c;

        consoleBuffer->head = (consoleBuffer->head + 1) % (consoleBuffer->size);
        V_kt_sem(nslots);
        count++;
    }
    V_kt_sem(readers);
    syscall_return(pcb, count);
}