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
#include <asm/errno.h>
#include "shm_malloc.h"
#include "sockets.h"
#include "benchmark.h"

#ifndef SHM_FILE
#define SHM_FILE "tshm_file"
#endif

#ifndef NUM_ITERS
#define NUM_ITERS 1000000
#endif

#define NUM_THREAD 4

pthread_barrier_t *barrier;

volatile size_lt *timer_ns;
volatile size_lt record[NUM_ITERS];

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
}

static inline size_lt stop_timer() {
//    *timer_ns[i] = elapsed_time_ns(*timer_ns[i]);
//    printf("Time taken: %llu ns | Per iter: %f ns\n", *timer_ns, ((double) *timer_ns) / NUM_ITERS);
    return elapsed_time_ns(*timer_ns);
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

int main() {
    shm_init(SHM_FILE, NULL);
    timer_ns = shm_malloc(sizeof(size_lt));
    barrier = shm_malloc(sizeof(pthread_barrier_t));
    pthread_barrierattr_t barattr;
    pthread_barrierattr_setpshared(&barattr, PTHREAD_PROCESS_SHARED);
    pthread_barrier_init(barrier, &barattr, NUM_THREAD * 2);
    pthread_barrierattr_destroy(&barattr);

    sock sock;
    init_socket(&sock);

    printf("\n======================initialized=====================\n");

    if (fork() == 0) {
        pthread_t s_threads[NUM_THREAD];
        for (int i = 0; i < NUM_THREAD; i++) {
            params p = {
                    .id = i,
                    .sock = sock
            };
            pthread_create(&s_threads[i], NULL, sender, &p);
            assign_thread_to_core(i, s_threads[i]);
            printf("Assigned sender core: %d\n", i);
        }
        for (int i = 0; i < NUM_THREAD; i++) {
            pthread_join(s_threads[i], NULL);
        }
        exit(0);
    }

    pthread_t r_threads[NUM_THREAD];
    for (int i = 0; i < NUM_THREAD; i++) {
        params p = {
                .id = i + NUM_THREAD,
                .sock = sock
        };
        pthread_create(&r_threads[i], NULL, receiver, &p);
        assign_thread_to_core(i + NUM_THREAD, r_threads[i]);
        printf("Assigned receiver core: %d\n", i + NUM_THREAD);
    }


    for (int i = 0; i < NUM_THREAD; i++) {
        pthread_join(r_threads[i], NULL);
    }
    int status;
    while (wait(&status) > 0);

    size_lt sum = 0;
    for (int i = 0; i < NUM_ITERS; i++) {
//        printf("\n%llu", record[i]);
        sum += record[i];
    }
    printf("Time taken: %llu ns | Per iter: %f ns\n", sum, ((double) sum) / NUM_ITERS);

    printf("\n=======================================================\n");
    pthread_barrier_destroy(barrier);
    shm_fini();
    shm_destroy();
}

static inline void *sender(void *par) {
    params *p = (params *) par;
    sock *s = &p->sock;
    int id = p->id;
    pthread_barrier_wait(barrier);
    for (int i = 0, msg = id; i < NUM_ITERS; i++, msg += NUM_THREAD) {
        start_timer();
        send_msg(s, msg);
    }
    return 0;
}

static inline void *receiver(void *par) {
    params *p = (params *) par;
    sock *s = &p->sock;
    int id = p->id;
    pthread_barrier_wait(barrier);
    for (int i = 0; i < NUM_ITERS; i++) {
        recv_msg(s);
        record[i] = stop_timer();
    }
    printf("Verify: %d\n", s->buf);
    return 0;
}
