#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "jrb.h"
#include "memory.h"
#include "kt.h"
#include "kos.h"
#include "dllist.h"

struct PCB {
    int registers[NumTotalRegs];
    void* sbrk_ptr;

    int mem_limit;
    int mem_base;

    unsigned short pid;
    int return_value;
    int mem_slot;

    struct PCB* parent;

    kt_sem waiters_sem;
    Dllist waiters;
    JRB children;

    struct FD *fd[64];
};

extern Dllist readyQueue;
extern struct PCB *running;

extern int currentPID;
extern JRB processTree;

extern bool is_noop;
extern struct PCB* init;

void initialize_scheduler();

void* initialize_user_process(void* arg);

void scheduler();

int perform_execve(struct PCB *pcb, char *filename, char **pcb_argv);

int get_new_pid();
void destroy_pid(int pid);

struct Pipe {
    char* buffer;

    int read_count;
    int write_count;

    kt_sem read;
    kt_sem write;
    kt_sem nelement;
    kt_sem space_available;

    int write_head;
    int read_head;

    int writer_in_use;
};

struct FD {
    bool console;
    bool open;
    bool is_read;
    int reference_count;
    struct Pipe* pipe;
};

#endif