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
#include "bits.h"
#include "cpumap.h"
#include "shm_malloc.h"

#ifndef NUM_ITERS
#define NUM_ITERS 1000000
#endif

#ifndef SHM_FILE
#define SHM_FILE "tshm6_file"
#endif

#define size_lt unsigned long long

static pthread_barrier_t *barrier;

size_lt *timer_us, *timer_ns;
static long long unsigned elapsed_time_ns(long long unsigned ns) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000000000L + t.tv_nsec - ns;
}

static long long unsigned elapsed_time_us(long long unsigned us) {
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



size_lt record[NUM_ITERS];

size_lt *starter, *stopper;

void thread_init(int id, int nprocs);

void *benchmark(int id, int nprocs);

void thread_exit(int id, int nprocs);

void init(int nprocs, int logn);

int verify(int nprocs, void **results);

void wfenqueue(int id, int value);

void *wfdequeue(int id);

static void *send(void *bits) {
    int id = bits_hi(bits);
    int nprocs = bits_lo(bits);
    cpu_set_t set;
    CPU_ZERO(&set);

    int cpu = cpumap(id, nprocs);
    CPU_SET(cpu, &set);
    sched_setaffinity(0, sizeof(set), &set);
    thread_init(id, nprocs);

    pthread_barrier_wait(barrier);

//    *starter = elapsed_time_us(0);
    start_timer();
    for (int i = 0; i < NUM_ITERS; i++) {
        wfenqueue(id, i);
    }

    pthread_barrier_wait(barrier);
    return 0;
}

static void *recv(void *bits) {
    int id = bits_hi(bits);
    int nprocs = bits_lo(bits);
    cpu_set_t set;
    CPU_ZERO(&set);

    int cpu = cpumap(id, nprocs);
    CPU_SET(cpu, &set);
    sched_setaffinity(0, sizeof(set), &set);
    thread_init(id, nprocs);

    pthread_barrier_wait(barrier);
    for (int i = 0; i < NUM_ITERS; i++) {
        void *result = wfdequeue(id);
    }
    stop_timer();
//    *stopper = elapsed_time_us(0);
    printf("\nTotal: %f", (double) elapsed_time_us(*starter)/NUM_ITERS);
    pthread_barrier_wait(barrier);
    return 0;
}

void cleanup(int nprocs);

int main(int argc, const char *argv[]) {
    int nprocs = 0;
    int n = 0;
    void *res;
//    nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    nprocs = 2;

    shm_init(SHM_FILE, NULL);

    timer_us = shm_malloc(sizeof(size_lt));
    timer_ns = shm_malloc(sizeof(size_lt));

    starter = shm_malloc(sizeof(size_lt));
    stopper = shm_malloc(sizeof(size_lt));

    size_lt *local_timer = shm_malloc(sizeof(size_lt));

    barrier = shm_malloc(sizeof(pthread_barrier_t));

    pthread_barrierattr_t barattr;
    pthread_barrierattr_setpshared(&barattr, PTHREAD_PROCESS_SHARED);
    pthread_barrier_init(barrier, &barattr, nprocs);
    pthread_barrierattr_destroy(&barattr);
    printf("=================================================\n");
    printf("  Number of processors: %d\n", nprocs);

    init(nprocs, n);

    if (fork() == 0) {
        pthread_t ths1;
        pthread_create(&ths1, NULL, recv, bits_join(1, nprocs));
        *local_timer = elapsed_time_us(0);
        pthread_join(ths1, NULL);

        size_lt avg = 0;
        for(int i = 0; i < NUM_ITERS; i++) {
            avg += record[i];
        }
        printf("\nAvg: %llu", avg/NUM_ITERS);
//        printf("\nAvg: %lf", (double)avg);
        cleanup(nprocs);
        exit(0);
    } else {
        pthread_t ths2;
        pthread_create(&ths2, NULL, send, bits_join(2, nprocs));
        pthread_join(ths2, res);
        *local_timer = elapsed_time_us(*local_timer);
        printf("\nTOTAL: %llu",*local_timer/NUM_ITERS);
    }
    int status;
    while (wait(&status) > 0);

     /*pthread_t ths1, ths2;

     pthread_create(&ths1, NULL, send, bits_join(1, nprocs));
     pthread_create(&ths2, NULL, recv, bits_join(2, nprocs));

     pthread_join(ths1, NULL);
     pthread_join(ths2, res);*/

    printf("===========================================\n");

    pthread_barrier_destroy(barrier);

    cleanup(nprocs);
    shm_fini();
    shm_destroy();
//    return verify(nprocs, res);
    return 0;
}