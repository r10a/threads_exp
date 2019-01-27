//
// Created by rohit on 1/26/2019.
//
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include <wait.h>
#include <string.h>
#include "shm_malloc.h"
#include "benchmark.h"

#ifndef SHM_FILE
#define SHM_FILE "tshm6_file"
#endif

#ifndef NUM_ITERS
#define NUM_ITERS 1000000
#endif

static pthread_barrier_t *barrier;

size_lt *timer_us, *timer_ns;
static size_lt elapsed_time_ns(size_lt ns) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000000000L + t.tv_nsec - ns;
}

static size_lt elapsed_time_us(size_lt us) {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000000 + t.tv_usec - us;
}
static inline void start_timer() {
    *timer_ns = elapsed_time_ns(0);
    *timer_us = elapsed_time_us(0);
}

static inline void stop_timer() {
    *timer_ns = elapsed_time_ns(*timer_ns);
    *timer_us = elapsed_time_us(*timer_us);
    printf("Time taken: %llu us | Per iter: %f us\n", *timer_us, ((double)*timer_us)/NUM_ITERS);
    printf("Time taken: %llu ns | Per iter: %f ns\n", *timer_ns, ((double)*timer_ns)/NUM_ITERS);
}


