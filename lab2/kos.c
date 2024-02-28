/*
 * kos.c -- starting point for student's os.
 *
 */
#include <stdlib.h>

#include "kt.h"
#include "kos.h"
#include "simulator.h"
#include "console_buf.h"
#include "scheduler.h"

kt_sem writers;
kt_sem readers;
kt_sem nelem;
kt_sem nslots;
kt_sem consoleWait;
kt_sem writeok;

void initialize_semaphores() {
    consoleWait = make_kt_sem(0);
    nelem = make_kt_sem(0);
    nslots = make_kt_sem(ConsoleBufferSize);

    writeok = make_kt_sem(0);
    writers = make_kt_sem(1);
    readers = make_kt_sem(1);
}

void KOS() {
    initialize_semaphores();
    initialize_scheduler();

    kt_fork(initialize_user_process, (void*) kos_argv);
    kt_fork((void*) console_reader_thread, NULL);
    scheduler();
}