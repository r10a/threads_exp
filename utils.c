//
// Created by rohit on 11/16/18.
//

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "utils.h"

int print(char *str) {
    write(1, str, strlen(str));
}

int print_int(int num) {
    char number[20];
    sprintf(number, "%d", num);
    print(number);
}

// Added from http://home.thep.lu.se/~bjorn/crc/
uint32_t crc32_for_byte(uint32_t r) {
    for(int j = 0; j < 8; ++j)
        r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
    return r ^ (uint32_t)0xFF000000L;
}

// Added from http://home.thep.lu.se/~bjorn/crc/
void crc32(const void *data, size_t n_bytes, uint32_t* crc) {
    static uint32_t table[0x100];
    if(!*table)
        for(size_t i = 0; i < 0x100; ++i)
            table[i] = crc32_for_byte(i);
    for(size_t i = 0; i < n_bytes; ++i)
        *crc = table[(uint8_t)*crc ^ ((uint8_t*)data)[i]] ^ *crc >> 8;
}
