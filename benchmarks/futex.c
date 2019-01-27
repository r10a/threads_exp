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

#define TV_1 "/TIME1"
#define TV_2 "/TIME2"
#define NUM_ITERS 1000

void *child();

void *parent();

int *futex;
#define size_lt unsigned long long

size_lt results[NUM_ITERS];
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
//    *timer_us = elapsed_time_us(0);
}

static inline size_lt stop_timer() {
//    *timer_ns = elapsed_time_ns(*timer_ns);
//    *timer_us = elapsed_time_us(*timer_us);
//    printf("Time taken: %llu us | Per iter: %f us\n", *timer_us, ((double)*timer_us)/NUM_ITERS);
//    printf("Time taken: %llu ns | Per iter: %f ns\n", *timer_ns, ((double)*timer_ns)/NUM_ITERS);
    return elapsed_time_ns(*timer_ns);
}


int main(void) {

    int t1, t2, m;

    timer_us = (size_lt *) mmap(NULL, sizeof(size_lt), PROT_READ | PROT_WRITE, MAP_SHARED,
                                create_shm(&t1, TV_1, sizeof(size_lt)), 0);
    timer_ns = (size_lt *) mmap(NULL, sizeof(size_lt), PROT_READ | PROT_WRITE, MAP_SHARED,
                                create_shm(&t2, TV_2, sizeof(size_lt)), 0);
    futex = (int *) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED,
                         create_shm(&m, "/msg", sizeof(int)), 0);
    *futex = 0xA;
    const pid_t other = fork();
    if (other == 0) {
        assign_curr_process_to_core(2);
        pthread_t thread;
        pthread_create(&thread, NULL, parent, NULL);
        assign_thread_to_core(2, thread);
        pthread_join(thread, NULL);
        exit(0);
    }
    assign_curr_process_to_core(1);
    pthread_t thread;
    pthread_create(&thread, NULL, child, NULL);
    assign_thread_to_core(1, thread);
    pthread_join(thread, NULL);


    size_lt sum = 0;
    for (int i = 0; i < NUM_ITERS; i++) {
        sum += results[i];
    }
    printf("Total: %llu | Mean: %f", sum, ((double) sum) / NUM_ITERS);

    wait(futex);
    munmap(futex, sizeof(int));
    munmap(timer_us, sizeof(size_lt));
    munmap(timer_ns, sizeof(size_lt));
    shm_unlink(TV_1);
    shm_unlink(TV_2);
    shm_unlink("/msg");
    return 0;
}

void *child() {
    for (int i = 0; i < NUM_ITERS; i++) {
        sched_yield();
        while (syscall(SYS_futex, futex, FUTEX_WAIT, 0xA, NULL, NULL, 42)) {
            // retry
            sched_yield();
        }
        results[i] = stop_timer();
        *futex = 0xB;
        while (!syscall(SYS_futex, futex, FUTEX_WAKE, 1, NULL, NULL, 42)) {
            // retry
            sched_yield();
        }
    }
    return 0;
}

void *parent() {
    for (int i = 0; i < NUM_ITERS; i++) {
        start_timer();
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