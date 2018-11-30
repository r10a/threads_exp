//
// Created by rohit on 11/16/18.
//

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "utils.h"
#include <errno.h>

int print(char *str) {
    write(1, str, strlen(str));
}

int print_int(int num) {
    char number[20];
    sprintf(number, "%d", num);
    print(number);
}

void fill_request(int shmid, size_t size) {
    void* request = mmap(NULL, size, PROT_WRITE, MAP_SHARED, shmid, 0);
    if(request == MAP_FAILED) {
        print("\nFailed: ");
        print_int(errno);
        return;
    }
//    char c;
//    char str[size];
//    rand_str(&c, sizeof(char));
//    memset(str, c, size);
//    memcpy(request, str, size);
//    msync(request, size, MS_SYNC);
    munmap(request, size);
}

void print_request(int shmid, size_t size) {
    char* request = mmap(NULL, size, PROT_READ, MAP_SHARED, shmid, 0);
    if(request == MAP_FAILED) {
        print("Failed");
        return;
    }
    mlock(request, size);
    print(request);
    munlock(request, size);
//    munmap(request, size);
}

void clear_request(int shmid, size_t size) {
    void* request = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0);
    print("here");
    print("\nshmid: ");
    print_int(shmid);
    print("\nsize: ");
    print_int(size);
    memset(request, 'w', size);
//    munmap(request, size);
}

// Added from https://stackoverflow.com/a/15768317/10660687
void rand_str(char *dest, size_t length) {
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}

size_t rand_size() {
    return (size_t) rand() % 40;
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
