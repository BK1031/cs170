//
// Created by Bharat Kathi on 2/27/24.
//

#include "memory.h"
int memory_partitions[8];

void initialize_memory() {
    for (int i = 0; i < 8; i++){
        memory_partitions[i] = 0;
    }
    User_Limit = MemorySize / 8;
}
