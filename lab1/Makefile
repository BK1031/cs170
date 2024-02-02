# Makefile for KOS -- the derivative of KOS (Jim Plank's OS)
# http://www.cs.utk.edu/~plank
#
# Michael A. Puening modified by Kim Buckner Spring 97
# further modified for kim's lab4
#
# modified for cs560 James Spring 00
#
# modified for UCSB, Spring 2002
#
# modified for UCSB further, Fall 2007 and 2008
#
# modified for UCSB further, Winter 2014


CC = gcc -m32
KHOME = /cs/faculty/rich/cs170
CFLAGS = -g -I$(KHOME)/include -I.
#LIBCVERSION = $(shell getconf GNU_LIBC_VERSION | grep glibc | sed 's/glibc \([0-9]*\.[0-9]*\).*/\1/')
#INCLUDES = -I$(KOSDIR)/include
LIB = $(KHOME)/lib/libsim.a $(KHOME)/lib/libkt.a $(KHOME)/lib/libfdr.a $(KHOME)/lib/libkos.a ${KHOME}/lib/libfdr.a
#LIB = -L$(KOSDIR)/lib -lsim -lkt$(LIBCVERSION) -lfdr -lnsl
MAIN = $(KHOME)/lib/main_lab1.o

EXECUTABLES = kos
USER_OBJS   = exception.o kos.o

all: $(EXECUTABLES)

kos: $(USER_OBJS) $(MAIN)
	$(CC) $(CFLAGS) -o kos $(MAIN) $(USER_OBJS) $(LIB)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $*.c

clean:
	/bin/rm -f *.o core $(EXECUTABLES)
