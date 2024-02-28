#ifndef CONSOLE_BUF_H
#define CONSOLE_BUF_H

#include "kt.h"

#define ConsoleBufferSize 256

extern struct buffer* consoleBuffer;

struct buffer {
    int* buff;
    int head;
    int tail;
    int size;
};

void console_reader_thread();

#endif