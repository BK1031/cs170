// Name: Bharat Kathi
// Date: 1/19/24
// Title: Lab0 - The MyMalloc() Lab
// **********************************************************************
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "my_malloc.h"


struct malloc_stc
{
        struct malloc_stc *next;
        struct malloc_stc *prev;
        int size;
        unsigned char *buffer;
};

static struct malloc_stc *MyFreeList;
static unsigned char MyBuff[MAX_MALLOC_SIZE];

void InitMyMalloc(void) {
    MyFreeList = (struct malloc_stc *)MyBuff;
    MyFreeList->next = NULL;
    MyFreeList->prev = NULL;
    MyFreeList->size = sizeof(MyBuff) - sizeof(struct malloc_stc);
    MyFreeList->buffer = (unsigned char *)(MyBuff + sizeof(struct malloc_stc));
}

void *MyMalloc(int size) {
    struct malloc_stc *curr = MyFreeList;
    struct malloc_stc *new;
    int roundedSize = (size + 7) & ~7;
    while (curr != NULL) {
        if (curr->size >= roundedSize) {
            if (curr->size - roundedSize > sizeof(struct malloc_stc)) {
                new = (struct malloc_stc *)(curr->buffer + roundedSize);
                new->next = curr->next;
                new->prev = curr;
                new->size = curr->size - roundedSize - sizeof(struct malloc_stc);
                new->buffer = curr->buffer + roundedSize + sizeof(struct malloc_stc);
                curr->next = new;
                curr->size = roundedSize;
            }
            if (curr->prev != NULL) {
                curr->prev->next = curr->next;
            }
            if (curr->next != NULL) {
                curr->next->prev = curr->prev;
            }
            return curr->buffer;
        }
        curr = curr->next;
    }
    return NULL;
}

void MyFree(void *ptr) {
    struct malloc_stc *curr = MyFreeList;
    struct malloc_stc *new;
    while (curr != NULL) {
        if (curr->buffer == ptr) {
            if (curr->prev != NULL) {
                curr->prev->next = curr->next;
            }
            if (curr->next != NULL) {
                curr->next->prev = curr->prev;
            }
            if (curr < MyFreeList) {
                MyFreeList = curr;
            }
            if (curr->next != NULL && curr->next->buffer == curr->buffer + curr->size) {
                curr->size += curr->next->size + sizeof(struct malloc_stc);
                curr->next = curr->next->next;
                if (curr->next != NULL) {
                    curr->next->prev = curr;
                }
            }
            if (curr->prev != NULL && curr->prev->buffer + curr->prev->size == curr->buffer) {
                curr->prev->size += curr->size + sizeof(struct malloc_stc);
                curr->prev->next = curr->next;
                if (curr->next != NULL) {
                    curr->next->prev = curr->prev;
                }
            }
            return;
        }
        curr = curr->next;
    }
}

void PrintMyMallocFreeList(void) {
    struct malloc_stc *curr = MyFreeList;
    while (curr != NULL) {
        printf("curr = %p, curr->next = %p, curr->prev = %p, curr->size = %d, curr->buffer = %p\n", curr, curr->next, curr->prev, curr->size, curr->buffer);
        curr = curr->next;
    }
}