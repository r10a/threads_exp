//
// Created by rohit on 12/2/2018.
//
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include "../utils.h"

#include "posix_mutex.h"

#define NUM_ITERS 100

#define TV_1 "/TIME1"
#define TV_2 "/TIME2"
#define TV_3 "/TIME3"

#define SHM "/shared_mem"

#define MUTEX_1 "/mutex"
#define CONDITION_1 "/cond"
#define MUTEX_2 "/mutex2"
#define CONDITION_2 "/cond2"

#define size_lt unsigned long long

static inline long long unsigned time_ns(struct timespec *const ts) {
    if (clock_gettime(CLOCK_MONOTONIC, ts)) {
        exit(1);
    }
    return ((long long unsigned) ts->tv_sec) * 1000000000LLU
           + (long long unsigned) ts->tv_nsec;
}

struct timespec ts;
long long unsigned *start_sh, *delta_sh;
char *mem;
pthread_mutex_t *mutex_1, *mutex_2;
pthread_cond_t *cond_1, *cond_2;

static inline void start_timer() {
    *start_sh = time_ns(&ts);
}

static inline void stop_timer() {
    *delta_sh = time_ns(&ts) - *start_sh;
    printf("Total: %llu | One transaction: %llu\n", *delta_sh, *delta_sh/NUM_ITERS);
}


void *send_message();

void *recv_message();

int main(int argc, char **argv) {


    int ts1, ts2, m, mutex, cond;
    int pid_1, pid_2;
    start_sh = (long long unsigned *) mmap(NULL, sizeof(long long unsigned), PROT_READ | PROT_WRITE, MAP_SHARED,
                                           create_shm(&ts1, "/time1", sizeof(long long unsigned)), 0);
    delta_sh = (long long unsigned *) mmap(NULL, sizeof(long long unsigned), PROT_READ | PROT_WRITE, MAP_SHARED,
                                           create_shm(&ts2, "/time2", sizeof(long long unsigned)), 0);
    mem = (char *) mmap(NULL, sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED,
                        create_shm(&m, SHM, sizeof(char)), 0);
    memset(mem, 'r', sizeof(char));

    mutex_1 = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED,
                                       create_shm(&mutex, MUTEX_1, sizeof(pthread_mutex_t)), 0);
    cond_1 = (pthread_cond_t *) mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED,
                                     create_shm(&cond, CONDITION_1, sizeof(pthread_cond_t)), 0);

    mutex_2 = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED,
                                       create_shm(&mutex, MUTEX_2, sizeof(pthread_mutex_t)), 0);
    cond_2 = (pthread_cond_t *) mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED,
                                     create_shm(&cond, CONDITION_2, sizeof(pthread_cond_t)), 0);

    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;

    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ADAPTIVE_NP);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex_1, &mattr);    /* Initialize mutex variable */
    pthread_mutex_init(mutex_2, &mattr);    /* Initialize mutex variable */

    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(cond_1, &cattr);  /* Initialize condition variable */
    pthread_cond_init(cond_2, &cattr);  /* Initialize condition variable */

    if ((pid_1 = fork()) == 0) {
        pthread_t thread;
        pthread_create(&thread, NULL, send_message, NULL);
        assign_thread_to_core(1, thread);
        int actual = get_core_number(thread);
        pthread_join(thread, NULL);
        exit(0);
    } else {
        pthread_t thread;
        pthread_create(&thread, NULL, recv_message, NULL);
        assign_thread_to_core(2, thread);
        int actual = get_core_number(thread);
        pthread_join(thread, NULL);
    }
    int pid, status;
    while ((pid = wait(&status)) > 0);

    munmap(start_sh, sizeof(long long unsigned));
    munmap(delta_sh, sizeof(long long unsigned));
    shm_unlink("/time1");
    shm_unlink("/time2");

    munmap(mem, sizeof(char));
    shm_unlink(SHM);

    pthread_condattr_destroy(&cattr);
    pthread_mutexattr_destroy(&mattr);
    pthread_mutex_destroy(mutex_1);    /* Free up the_mutex */
    pthread_cond_destroy(cond_1);        /* Free up consumer condition variable */
    pthread_mutex_destroy(mutex_2);    /* Free up the_mutex */
    pthread_cond_destroy(cond_2);        /* Free up consumer condition variable */
    munmap(mutex_1, sizeof(pthread_mutex_t));
    munmap(cond_1, sizeof(pthread_cond_t));
    munmap(mutex_2, sizeof(pthread_mutex_t));
    munmap(cond_2, sizeof(pthread_cond_t));
    shm_unlink(MUTEX_1);
    shm_unlink(CONDITION_1);
    shm_unlink(MUTEX_2);
    shm_unlink(CONDITION_2);
}

void *send_message() {
    start_timer();
    for (int i = 0; i < NUM_ITERS; i++) {
        pthread_mutex_lock(mutex_1);
//        printf("\nSend locked");
        while (*mem != 'w')
            pthread_cond_wait(cond_2, mutex_1);
        memset(mem, 'r', sizeof(char));
        pthread_cond_signal(cond_1);
        pthread_mutex_unlock(mutex_1);
//        printf("\nSend unlocked");
    }
}

void *recv_message() {
    for (int i = 0; i < NUM_ITERS; i++) {
        pthread_mutex_lock(mutex_1);
//        printf("\nRecv locked");
        while (*mem != 'r')
            pthread_cond_wait(cond_1, mutex_1);
//        printf(mem);
        memset(mem, 'w', sizeof(char));
        pthread_cond_signal(cond_2);
        pthread_mutex_unlock(mutex_1);
//        printf("\nRecv unlocked");
    }
    stop_timer();
}