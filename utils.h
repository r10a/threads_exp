//
// Created by rohit on 11/16/18.
//

#ifndef THREADS_EXP_UTILS_H
#define THREADS_EXP_UTILS_H

#endif //THREADS_EXP_UTILS_H

int print(char *);

int print_int(int);

// TODO: Use crc32 in Process 2
// Added from http://home.thep.lu.se/~bjorn/crc/
void crc32(const void *data, size_t n_bytes, uint32_t* crc);

// TODO: Allow variable size requests
typedef struct request {
    void *address;
    int size;
} request;