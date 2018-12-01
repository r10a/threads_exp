//
// Created by rohit on 11/16/18.
//

#include <stdint.h>

#ifndef THREADS_EXP_UTILS_H
#define THREADS_EXP_UTILS_H

#endif //THREADS_EXP_UTILS_H

int print(char *);

int print_int(int);

void fill_request(char [], size_t);

void print_request(char [], size_t);

size_t rand_size();

int create_shm(int *share, char *path, size_t size);

int open_shm(int *share, char *path, size_t size);

void rand_str(char *dest, size_t length);

// TODO: Use crc32 in Process 2
// Added from http://home.thep.lu.se/~bjorn/crc/
void crc32(const void *data, size_t n_bytes, uint32_t* crc);

// TODO: Allow variable size requests
typedef struct request {
    size_t size;
    char shmnm[10];
} request;