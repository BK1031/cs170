#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "dllist.h"
#include "simulator.h"
#include "kt.h"

#define ReadBufferSize 256

struct PCB {
    int registers[NumTotalRegs];
};

extern Dllist readyQueue;
extern struct PCB *running;

extern int readBuffer[ReadBufferSize];
extern int readHead;
extern int readTail;

extern kt_sem write_ok;
extern kt_sem writers;
extern kt_sem nelem;
extern kt_sem nslots;
extern kt_sem consoleWait;

void scheduler(void);

#endif // SCHEDULER_H