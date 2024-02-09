#include "console_buf.h"
#include "kt.h"
#include "simulator.h"
#include "kos.h"


int readBuffer[ReadBufferSize];
int readHead = 0;
int readTail = 0;

void console_reader_thread() {
    while (1) {
        P_kt_sem(consoleWait);
        P_kt_sem(nslots);

        char c = console_read();

        readBuffer[readTail] = c;
        readTail = (readTail + 1) % ReadBufferSize;

        V_kt_sem(nelem);
    }
}