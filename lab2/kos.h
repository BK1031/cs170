#ifndef KOS_H
#define KOS_H

#include "kt.h"
#include "simulator.h"

extern kt_sem writers;
extern kt_sem readers;
extern kt_sem nelem;
extern kt_sem nslots;
extern kt_sem consoleWait;
extern kt_sem writeok;

void KOS();

#endif // KOS_H