#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include <wait.h>
#include "bits.h"
#include "cpumap.h"
#include "shm_malloc.h"

#ifndef NUM_ITERS
#define NUM_ITERS 5
#endif

#ifndef MAX_ITERS
#define MAX_ITERS 20
#endif

#ifndef SHM_FILE
#define SHM_FILE "tshm3_file"
#endif

static pthread_barrier_t *barrier;
static volatile int target;

//size_t *timer;
struct timespec ts;
static size_t elapsed_time(size_t us) {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000000 + t.tv_usec - us;
}

long long unsigned *start_sh, *delta_sh;
static inline long long unsigned time_ns(struct timespec *const ts) {
    if (clock_gettime(CLOCK_MONOTONIC, ts)) {
        exit(1);
    }
    return ((long long unsigned) ts->tv_sec) * 1000000000LLU
           + (long long unsigned) ts->tv_nsec;
}
static inline void start_timer() {
    *start_sh = time_ns(&ts);
}

static inline void stop_timer() {
    *delta_sh = time_ns(&ts) - *start_sh;
    printf("%llu\n", *delta_sh / 1000);
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

    for (int i = 0; i < 1000; i++) {
        printf("\n Enqueued: %d", i);
        pthread_barrier_wait(barrier);

        wfenqueue(id, i);
//        *timer = elapsed_time(0);
        start_timer();
    }
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

    for (int i = 0; i < 1000; i++) {
        pthread_barrier_wait(barrier);
        void *result = wfdequeue(id);
//        *timer = elapsed_time(*timer);
        printf("\n Dequeued: %ld\n", (intptr_t) result);
        stop_timer();
    }
    thread_exit(id, nprocs);
    return 0;
}

int main(int argc, const char *argv[]) {
    int nprocs = 0;
    int n = 0;
    void *res;
//    nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    nprocs = 2;

    shm_init(SHM_FILE, NULL);

//    timer = shm_malloc(sizeof(size_t));
//    timer = shm_malloc(sizeof(struct timespec));
    start_sh = shm_malloc(sizeof(long long unsigned));
    delta_sh = shm_malloc(sizeof(long long unsigned));

    barrier = shm_malloc(sizeof(pthread_barrier_t));

    pthread_barrierattr_t barattr;
    pthread_barrierattr_setpshared(&barattr, PTHREAD_PROCESS_SHARED);
    pthread_barrier_init(barrier, &barattr, nprocs);
    pthread_barrierattr_destroy(&barattr);
    printf("=================================================\n");
    printf("  Number of processors: %d\n", nprocs);

    init(nprocs, n);

    if (fork() == 0) {
//        shm_child();
        pthread_t ths1;
        pthread_create(&ths1, NULL, recv, bits_join(1, nprocs));
        pthread_join(ths1, NULL);
        exit(0);
    } else {
        pthread_t ths2;
        pthread_create(&ths2, NULL, send, bits_join(2, nprocs));
        pthread_join(ths2, res);
    }
    int status;
    while (wait(&status) > 0);

    /* pthread_t ths1, ths2;

     pthread_create(&ths1, NULL, send, bits_join(0, nprocs));
     pthread_create(&ths2, NULL, recv, bits_join(1, nprocs));

     pthread_join(ths1, NULL);
     pthread_join(ths2, res);*/

    printf("===========================================\n");

    pthread_barrier_destroy(barrier);

    shm_destroy();
//    return verify(nprocs, res);
    return 0;
}