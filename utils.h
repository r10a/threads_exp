//
// Created by rohit on 11/16/18.
//

#include <stdint.h>

#ifndef THREADS_EXP_UTILS_H
#define THREADS_EXP_UTILS_H

#endif //THREADS_EXP_UTILS_H

int print(char *);

int print_int(int);

void fill_request(int, size_t);

void print_request(int, size_t);

void clear_request(int, size_t);

size_t rand_size();

void rand_str(char *dest, size_t length);

// TODO: Use crc32 in Process 2
// Added from http://home.thep.lu.se/~bjorn/crc/
void crc32(const void *data, size_t n_bytes, uint32_t* crc);

// TODO: Allow variable size requests
typedef struct request {
    int shmid;
    size_t size;
} request;