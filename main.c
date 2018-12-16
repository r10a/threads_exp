#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include "bits.h"
#include "cpumap.h"

#ifndef NUM_ITERS
#define NUM_ITERS 5
#endif

#ifndef MAX_ITERS
#define MAX_ITERS 20
#endif

static pthread_barrier_t barrier;
static volatile int target;

static size_t elapsed_time(size_t us) {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000000 + t.tv_usec - us;
}

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
    pthread_barrier_wait(&barrier);
    int item = 1;
    printf("\n Enqueued: %d", item);
    wfenqueue(id, item);

    thread_exit(id, nprocs);
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
    pthread_barrier_wait(&barrier);
//    wfenqueue(id);

    void *result = wfdequeue(id);

    printf("\n Dequeued: %ld\n", (intptr_t)result);

    thread_exit(id, nprocs);
    return result;
}


static void *thread(void *bits) {
    int id = bits_hi(bits);
    int nprocs = bits_lo(bits);
    cpu_set_t set;
    CPU_ZERO(&set);

    int cpu = cpumap(id, nprocs);
    CPU_SET(cpu, &set);
    sched_setaffinity(0, sizeof(set), &set);

    thread_init(id, nprocs);
    pthread_barrier_wait(&barrier);
    wfenqueue(id, 20);

    void *result = wfdequeue(id);

    thread_exit(id, nprocs);
    return result;
}

int main(int argc, const char *argv[]) {
    int nprocs = 0;
    int n = 0;

//    nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    nprocs = 2;

    pthread_barrier_init(&barrier, NULL, nprocs);
    printf("=================================================\n");
    printf("  Number of processors: %d\n", nprocs);

    init(nprocs, n);

    pthread_t ths1, ths2;
    void *res;
    pthread_create(&ths1, NULL, send, bits_join(0, nprocs));
    pthread_create(&ths2, NULL, recv, bits_join(1, nprocs));

    pthread_join(ths1, NULL);
    pthread_join(ths2, res);

    printf("===========================================\n");

    pthread_barrier_destroy(&barrier);
    return verify(nprocs, res);
}