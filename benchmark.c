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
#include "pipe.h"
#include "benchmark.h"

#ifndef SHM_FILE
#define SHM_FILE "tshm_file"
#endif

#ifndef NUM_ITERS
#define NUM_ITERS 1000000
#endif

#ifndef NUM_RUNS
#define NUM_RUNS 3
#endif

#define NUM_THREAD 1

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
    pipe_t pipe12, pipe23, pipe32, pipe21;
    init_pipe(&pipe12);
    init_pipe(&pipe23);
    init_pipe(&pipe32);
    init_pipe(&pipe21);

    /** One socket per thread
     * */
    /*pipe_t pipe12[NUM_THREAD], pipe23[NUM_THREAD], pipe32[NUM_THREAD], pipe21[NUM_THREAD];
    for(int i = 0; i < NUM_THREAD; i++) {
        init_pipe(&pipe12[i]);
        init_pipe(&pipe23[i]);
        init_pipe(&pipe32[i]);
        init_pipe(&pipe21[i]);
    }*/

    printf("\n======================initialized=====================\n");

    params p[NUM_THREAD];
    for(int i = 0; i < NUM_THREAD; i++) {
        p[i].id = i;
        p[i].pipe12 = pipe12;
        p[i].pipe23 = pipe23;
        p[i].pipe32 = pipe32;
        p[i].pipe21 = pipe21;
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
            avg += p[i].pipe21.buf;
            min = min > p[i].pipe21.buf ? p[i].pipe21.buf : min;
            max = max > p[i].pipe21.buf ? max : p[i].pipe21.buf;
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


//		printf("Time taken: %llu ns | Per iter: %f ns | Max: %llu | Min: %llu\n", sum, ((double) sum) / NUM_ITERS, max, min);
    printf("\n=======================================================\n");
    pthread_barrier_destroy(tmp_barrier);
    pthread_barrier_destroy(barrier);
    shm_fini();
    shm_destroy();
}

static void *sender(void *par) {
    params *p = (params *) par;
    pipe_t *p12 = &p->pipe12;
    pipe_t *p23 = &p->pipe23;
    pipe_t *p32 = &p->pipe32;
    pipe_t *p21 = &p->pipe21;
    printf("Senders: %d %d %d %d \n", p12->p[0], p12->p[1], p21->p[0], p21->p[1]);
    int id = p->id;
	size_lt start[NUM_RUNS];
    size_lt end[NUM_RUNS];
    size_lt delay[NUM_RUNS];
    size_lt time[NUM_RUNS];

    close(p12->p[0]);
    close(p21->p[1]);
    close(p23->p[0]);
    close(p23->p[1]);
    close(p32->p[0]);
    close(p32->p[1]);

    for(int j = 0; j < NUM_RUNS; j++) {
        pthread_barrier_wait(barrier);

        delay[j] = elapsed_time_ns(0);
        start[j] = elapsed_time_ns(0);
        for (int i = 0; i < NUM_ITERS; i++) {
            write(p12->p[1], &i, sizeof(p12->buf));
            read(p21->p[0], &p21->buf, sizeof(p21->buf));
        }
        end[j] = elapsed_time_ns(0);

        size_lt timer_overhead = start[j] - delay[j];
        time[j] = end[j] - start[j] - timer_overhead;
        printf("Time taken for #%d: %llu ns | Per iter: %f ns | ID: %d\n", j, time[j], ((double) time[j]) / NUM_ITERS, id);
        printf("Verify S: %llu\n", p21->buf);
    }
    size_lt avg = 0;
    for(int j = 0; j < NUM_RUNS; j++) {
        avg += llround(((double) time[j]) / NUM_ITERS);
    }
    p21->buf = llround(((double) avg) / NUM_RUNS);
    return 0;
}

static void *intermediate(void *par) {
    params *p = (params *) par;
    pipe_t *p12 = &p->pipe12;
    pipe_t *p23 = &p->pipe23;
    pipe_t *p32 = &p->pipe32;
    pipe_t *p21 = &p->pipe21;
    printf("Intermediates: %d %d %d %d %d %d %d %d\n", p12->p[0], p12->p[1], p23->p[0], p23->p[1], p32->p[0], p32->p[1], p21->p[0], p21->p[1]);

    close(p12->p[1]);
    close(p21->p[0]);
    close(p23->p[0]);
    close(p32->p[1]);

    int id = p->id;
     for(int j = 0; j < NUM_RUNS; j++) {
        pthread_barrier_wait(barrier);

        for (int i = 0; i < NUM_ITERS; i++) {
            read(p12->p[0], &p12->buf, sizeof(p12->buf));
            write(p23->p[1], &p12->buf, sizeof(p12->buf));
            read(p32->p[0], &p32->buf, sizeof(p32->buf));
            write(p21->p[1], &p32->buf, sizeof(p32->buf));
        }
    }
    printf("Intermediate completed\n");
//    printf("Verify I: %d\n", s23->buf);
    return 0;
}

static void *receiver(void *par) {
    params *p = (params *) par;
    pipe_t *p12 = &p->pipe12;
    pipe_t *p23 = &p->pipe23;
    pipe_t *p32 = &p->pipe32;
    pipe_t *p21 = &p->pipe21;
    printf("Receivers: %d %d %d %d \n", p23->p[0], p23->p[1], p32->p[0], p32->p[1]);
    int id = p->id;
    close(p12->p[0]);
    close(p12->p[1]);
    close(p21->p[0]);
    close(p21->p[1]);
    close(p23->p[1]);
    close(p32->p[0]);

    for(int j = 0; j < NUM_RUNS; j++) {
        pthread_barrier_wait(barrier);
        for (int i = 0; i < NUM_ITERS; i++) {
            read(p23->p[0], &p23->buf, sizeof(p23->buf));
            write(p32->p[1], &p23->buf, sizeof(p23->buf));
        }
    }
    printf("Receiver completed\n");
//    printf("Verify R: %d\n", s23->buf);
    return 0;
}