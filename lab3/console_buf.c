#include <stdlib.h>
#include "simulator.h"
#include "console_buf.h"
#include "kt.h"
#include "kos.h"

struct buffer* consoleBuffer;

struct buffer* create_buffer() {
    struct buffer* b;
    b = malloc(sizeof(struct buffer));
    b->buff = malloc(ConsoleBufferSize*sizeof(int));
    b->head = 0;
    b->tail = 0;
    b->size = 256;
    return b;
}

void initialize_console() {
    consoleBuffer = create_buffer();
}

void console_reader_thread() {
    initialize_console();
    while (1) {
        P_kt_sem(consoleWait);
        P_kt_sem(nslots);
        char c = (char)console_read();
        V_kt_sem(nelem);
        consoleBuffer->buff[(consoleBuffer->tail) % (consoleBuffer->size)] = c;
        consoleBuffer->tail = (consoleBuffer->tail + 1) % (consoleBuffer->size);
    }
}