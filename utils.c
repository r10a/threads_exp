//
// Created by rohit on 11/16/18.
//
#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "utils.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>

int print(char *str) {
    write(1, str, strlen(str));
}

int print_int(int num) {
    char number[20];
    sprintf(number, "%d", num);
    print(number);
}

int print_double(double num) {
    char number[20];
    sprintf(number, "%lf", num);
    print(number);
}

void fill_request(char shmnm[], size_t size) {
    int shmid;
    char *request = mmap(NULL, size, PROT_WRITE, MAP_SHARED, create_shm(&shmid, shmnm, size), 0);
    if (request == MAP_FAILED) {
        print("failure on mmap");
        print_int(errno);
        return;
    }
    memset(request, 'r', size);
    munmap(request, size);
}

void print_request(char shmnm[], size_t size) {
    int shmid;
    char *request = mmap(NULL, size, PROT_READ, MAP_SHARED, open_shm(&shmid, shmnm, size), 0);
    if (request == MAP_FAILED) {
        print("failure on mmap: ");
        print_int(errno);
        return;
    }
    print(request);
    munmap(request, size);
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

int create_shm(int *share, char *path, size_t size) {
    shm_unlink(path);
    *share = shm_open(path, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG);

    if (*share < 0) {
        print("failure on shm_open: ");
        print_int(errno);
        exit(1);
    }
    if (ftruncate(*share, size) == -1) {
        print("Error on ftruncate\n");
        exit(-1);
    }
    return *share;
}

int open_shm(int *share, char *path, size_t size) {
    *share = shm_open(path, O_RDWR, S_IRWXU | S_IRWXG);

    if (*share < 0) {
        print("failure on shm_open");
        exit(1);
    }
    if (ftruncate(*share, size) == -1) {
        print("Error on ftruncate\n");
        exit(-1);
    }
    return *share;
}

int assign_thread_to_core(int core_id, pthread_t pthread) {
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (core_id < 0 || core_id >= num_cores)
        return EINVAL;

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    return pthread_setaffinity_np(pthread, sizeof(cpu_set_t), &cpuset);
}

int get_core_number(pthread_t thread) {
    cpu_set_t cpuset;
    pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    for (int j = 0; j < CPU_SETSIZE; j++) {
        if (CPU_ISSET(j, &cpuset)) {
            return j;
        }
    }
    return -1;
}


// Added from http://home.thep.lu.se/~bjorn/crc/
uint32_t crc32_for_byte(uint32_t r) {
    for (int j = 0; j < 8; ++j)
        r = (r & 1 ? 0 : (uint32_t) 0xEDB88320L) ^ r >> 1;
    return r ^ (uint32_t) 0xFF000000L;
}

// Added from http://home.thep.lu.se/~bjorn/crc/
void crc32(const void *data, size_t n_bytes, uint32_t *crc) {
    static uint32_t table[0x100];
    if (!*table)
        for (size_t i = 0; i < 0x100; ++i)
            table[i] = crc32_for_byte(i);
    for (size_t i = 0; i < n_bytes; ++i)
        *crc = table[(uint8_t) *crc ^ ((uint8_t *) data)[i]] ^ *crc >> 8;
}
