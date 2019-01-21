#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/futex.h>
#include <sys/mman.h>
#include <pthread.h>
#include <bits/time.h>
#include <time.h>
#include <sys/time.h>
#include "../utils.h"

#define NUM_ITERS 1000

static inline long long unsigned time_ns(struct timespec *const ts) {
    if (clock_gettime(CLOCK_REALTIME, ts)) {
        exit(1);
    }
    return ((long long unsigned) ts->tv_sec) * 1000000000LLU
           + (long long unsigned) ts->tv_nsec;
}

static long long unsigned elapsed_time(long long unsigned ns) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000000000L + t.tv_nsec - ns;
}

static long long unsigned elapsed_time_us(long long unsigned us) {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000000 + t.tv_usec - us;
}

void *child();

void *parent();

struct timespec ts;
long long unsigned *start_sh, *delta_sh;
int *futex;
const int iterations = NUM_ITERS;

static inline void start_timer() {
    *start_sh = elapsed_time_us(0);
}

static inline void stop_timer() {
    *start_sh = elapsed_time_us(*start_sh);
    printf("%llu\n", *start_sh/NUM_ITERS);
}

int main(void) {

    int ts1, m, ts2;
    start_sh = (long long unsigned *) mmap(NULL, sizeof(long long unsigned), PROT_READ | PROT_WRITE, MAP_SHARED,
                                           create_shm(&ts1, "/time1", sizeof(long long unsigned)), 0);
    delta_sh = (long long unsigned *) mmap(NULL, sizeof(long long unsigned), PROT_READ | PROT_WRITE, MAP_SHARED,
                                           create_shm(&ts2, "/time2", sizeof(long long unsigned)), 0);
    futex = (int *) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED,
                         create_shm(&m, "/msg", sizeof(int)), 0);
    const pid_t other = fork();
    *futex = 0xA;
    if (other == 0) {
        assign_curr_process_to_core(1);
        pthread_t thread;
        pthread_create(&thread, NULL, child, NULL);
        assign_thread_to_core(1, thread);
        pthread_join(thread, NULL);
        exit(0);
    }

    assign_curr_process_to_core(2);
    const long long unsigned start_ns = time_ns(&ts);
    pthread_t thread;
    pthread_create(&thread, NULL, parent, NULL);
    assign_thread_to_core(2, thread);
    pthread_join(thread, NULL);
    const long long unsigned delta = time_ns(&ts) - start_ns;

    printf("Time taken: %lluns (%.1f ns/msg)\n", delta, ((float)delta / iterations));
    wait(futex);
    munmap(start_sh, sizeof(long long unsigned));
    munmap(delta_sh, sizeof(long long unsigned));
    munmap(futex, sizeof(int));
    shm_unlink("/time1");
    shm_unlink("/time2");
    shm_unlink("/msg");
    return 0;
}

void *child() {
    for (int i = 0; i < iterations; i++) {
        sched_yield();
        while (syscall(SYS_futex, futex, FUTEX_WAIT, 0xA, NULL, NULL, 42)) {
            // retry
            sched_yield();
        }
        *futex = 0xB;
        while (!syscall(SYS_futex, futex, FUTEX_WAKE, 1, NULL, NULL, 42)) {
            // retry
            sched_yield();
        }
    }
    stop_timer();
    return 0;
}

void *parent() {
    start_timer();
    for (int i = 0; i < iterations; i++) {
        *futex = 0xA;
        while (!syscall(SYS_futex, futex, FUTEX_WAKE, 1, NULL, NULL, 42)) {
            // retry
            sched_yield();
        }
        sched_yield();
        while (syscall(SYS_futex, futex, FUTEX_WAIT, 0xB, NULL, NULL, 42)) {
            // retry
            sched_yield();
        }
    }
    return 0;
}