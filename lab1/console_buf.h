#ifndef CONSOLE_BUF_H
#define CONSOLE_BUF_H

#define ReadBufferSize 256

extern int readBuffer[ReadBufferSize];
extern int readHead;
extern int readTail;

void console_reader_thread();

#endif // CONSOLE_BUF_H