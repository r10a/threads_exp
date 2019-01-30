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

#ifndef NUM_RUNS
#define NUM_RUNS 5
#endif

#define NUM_THREAD 8

pthread_barrier_t *barrier, *tmp_barrier;

volatile size_lt *start_g, *end_g, *delay_g;

static inline size_lt elapsed_time_ns(size_lt ns) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000000000L + t.tv_nsec - ns;
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
    barrier = shm_malloc(sizeof(pthread_barrier_t));
    tmp_barrier = shm_malloc(sizeof(pthread_barrier_t));
    start_g = shm_malloc(sizeof(size_lt)*NUM_ITERS);
    end_g = shm_malloc(sizeof(size_lt)*NUM_ITERS);
    delay_g = shm_malloc(sizeof(size_lt)*NUM_ITERS);

    pthread_barrierattr_t barattr;
    pthread_barrierattr_setpshared(&barattr, PTHREAD_PROCESS_SHARED);
    pthread_barrier_init(barrier, &barattr, NUM_THREAD * 3);
    pthread_barrier_init(tmp_barrier, &barattr, NUM_THREAD * 2);
    pthread_barrierattr_destroy(&barattr);

    /** One socket
     * */
    sock sock12, sock23;
    init_socket(&sock12);
    init_socket(&sock23);

    /** One socket per thread
     * */
    /*sock sock12[NUM_THREAD], sock23[NUM_THREAD];
    for(int i = 0; i < NUM_THREAD; i++) {
        init_socket(&sock12[i]);
        init_socket(&sock23[i]);
    }*/

    printf("\n======================initialized=====================\n");

    params p[NUM_THREAD];
    for(int i = 0; i < NUM_THREAD; i++) {
        p[i].id = i;
        p[i].sock12 = sock12;
        p[i].sock23 = sock23;
    }

    if (fork() == 0) {
        pthread_t s_threads[NUM_THREAD];
        for (int i = 0; i < NUM_THREAD; i++) {
            pthread_create(&s_threads[i], NULL, sender, &p[i]);
            assign_thread_to_core(i, s_threads[i]);
//            printf("Assigned sender core: %d\n", i);
        }
        for (int i = 0; i < NUM_THREAD; i++) {
            pthread_join(s_threads[i], NULL);
        }

        size_lt avg = 0, min = INT_MAX, max = 0;
        for (int i = 0; i < NUM_THREAD; i++) {
            avg += p[i].sock12.buf;
            min = min > p[i].sock12.buf ? p[i].sock12.buf : min;
            max = max > p[i].sock12.buf ? max : p[i].sock12.buf;
        }
        printf("Avg time taken: %f ns | Max: %llu | Min: %llu\n", ((double) avg) / NUM_THREAD, max, min);
        printf("Process 1 completed\n");
        exit(0);
    }

    if (fork() == 0) {
        pthread_t i_threads[NUM_THREAD];
        for (int i = 0; i < NUM_THREAD; i++) {
            pthread_create(&i_threads[i], NULL, intermediate, &p[i]);
            assign_thread_to_core(i + NUM_THREAD*2, i_threads[i]);
//            assign_thread_to_core(i, i_threads[i]);
//            printf("Assigned intermediate core: %d\n", i + NUM_THREAD*2);
        }
        for (int i = 0; i < NUM_THREAD; i++) {
            pthread_join(i_threads[i], NULL);
        }
        printf("Process 2 completed\n");
        exit(0);
    }

    if (fork() == 0) {
        pthread_t r_threads[NUM_THREAD];
        for (int i = 0; i < NUM_THREAD; i++) {
            pthread_create(&r_threads[i], NULL, receiver, &p[i]);
            assign_thread_to_core(i + NUM_THREAD, r_threads[i]);
//            printf("Assigned receiver core: %d\n", i + NUM_THREAD);
        }
        for (int i = 0; i < NUM_THREAD; i++) {
            pthread_join(r_threads[i], NULL);
        }
        printf("Process 3 completed\n");
        exit(0);
    }

    int status;
    while (wait(&status) > 0) printf("Process exit status: %d\n", status);

//    printf("Time taken: %llu ns | Per iter: %f ns | Max: %llu | Min: %llu\n", sum, ((double) sum) / NUM_ITERS, max, min);
    printf("\n=======================================================\n");
    pthread_barrier_destroy(tmp_barrier);
    pthread_barrier_destroy(barrier);
    shm_fini();
    shm_destroy();
}

static void *sender(void *par) {
    params *p = (params *) par;
    sock *s = &p->sock12;
    printf("Senders: %d %d \n", s->sv[0], s->sv[1]);
    int id = p->id;
    size_lt start[NUM_RUNS];
    size_lt end[NUM_RUNS];
    size_lt delay[NUM_RUNS];
    size_lt time[NUM_RUNS];

    for(int j = 0; j < NUM_RUNS; j++) {
        pthread_barrier_wait(barrier);

        delay[j] = elapsed_time_ns(0);
        start[j] = elapsed_time_ns(0);
        for (int i = 0; i < NUM_ITERS; i++) {
            write(s->sv[0], &i, sizeof(s->buf));
            read(s->sv[0], &s->buf, sizeof(s->buf));
        }
        end[j] = elapsed_time_ns(0);

        size_lt timer_overhead = start[j] - delay[j];
        time[j] = end[j] - start[j] - timer_overhead;
        printf("Time taken for #%d: %llu ns | Per iter: %f ns | ID: %d\n", j, time[j], ((double) time[j]) / NUM_ITERS, id);
        printf("Verify S: %llu\n", s->buf);
    }
    size_lt avg = 0;
    for(int j = 0; j < NUM_RUNS; j++) {
        avg += llround(((double) time[j]) / NUM_ITERS);
    }
    s->buf = llround(((double) avg) / NUM_RUNS);
    return 0;
}

static void *intermediate(void *par) {
    params *p = (params *) par;
    sock *s12 = &p->sock12;
    sock *s23 = &p->sock23;
    printf("Intermediates: %d %d %d %d\n", s12->sv[0], s12->sv[1], s23->sv[0], s23->sv[1]);
    int id = p->id;
    for(int j = 0; j < NUM_RUNS; j++) {
        pthread_barrier_wait(barrier);

        for (int i = 0; i < NUM_ITERS; i++) {
            read(s12->sv[1], &s12->buf, sizeof(s12->buf));
            write(s23->sv[0], &s12->buf, sizeof(s12->buf));
            read(s23->sv[0], &s23->buf, sizeof(s23->buf));
            write(s12->sv[1], &s23->buf, sizeof(s23->buf));
        }
    }
    printf("Intermediate completed\n");
//    printf("Verify I: %d\n", s23->buf);
    return 0;
}

static void *receiver(void *par) {
    params *p = (params *) par;
    sock *s23 = &p->sock23;
    printf("Receivers: %d %d \n", s23->sv[0], s23->sv[1]);
    int id = p->id;
    for(int j = 0; j < NUM_RUNS; j++) {
        pthread_barrier_wait(barrier);
        for (int i = 0; i < NUM_ITERS; i++) {
            read(s23->sv[1], &s23->buf, sizeof(s23->buf));
            write(s23->sv[1], &s23->buf, sizeof(s23->buf));
        }
    }
    printf("Receiver completed\n");
//    printf("Verify R: %d\n", s23->buf);
    return 0;
}
