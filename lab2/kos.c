/*
 * kos.c -- starting point for student's os.
 *
 */
#include <stdlib.h>

#include "kos.h"
#include "simulator.h"
#include "console_buf.h"
#include "scheduler.h"
#include "kt.h"

kt_sem write_ok;
kt_sem writers;
kt_sem nelem;
kt_sem nslots;
kt_sem consoleWait;

void initialize_semaphores() {
    // Initialize console write buffer
    write_ok = make_kt_sem(0);
    writers = make_kt_sem(1);

    // Initialize console read buffer
    consoleWait = make_kt_sem(0);
    nelem = make_kt_sem(0);
    nslots = make_kt_sem(ReadBufferSize);
}

void KOS() {
    initialize_semaphores();
    initialize_scheduler();
    kt_fork(initialize_user_process, kos_argv);
    kt_fork((void*) console_reader_thread, NULL);
    scheduler();
}