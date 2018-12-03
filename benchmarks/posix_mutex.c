//
// Created by rohit on 12/2/2018.
//

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

#define TV_1 "/TIME1"
#define TV_2 "/TIME2"
#define TV_3 "/TIME3"

#define SHM "/shared_mem"

#define MUTEX_1 "/mutex"
#define CONDITION_1 "/cond"

struct timeval *tv1, *tv2, *tv3;
char *mem;
pthread_mutex_t *mutex_1;
pthread_cond_t *cond_1;

void start_timer();

void stop_timer();

void *send_message();

void *recv_message();

int main(int argc, char **argv) {

    for (int i = 0; i < 100; i++) {

        int t1, t2, t3, m, mutex, cond;
        int pid_1, pid_2;
        tv1 = (struct timeval *) mmap(NULL, sizeof(struct timeval), PROT_READ | PROT_WRITE, MAP_SHARED,
                                      create_shm(&t1, TV_1, sizeof(struct timeval)), 0);
        tv2 = (struct timeval *) mmap(NULL, sizeof(struct timeval), PROT_READ | PROT_WRITE, MAP_SHARED,
                                      create_shm(&t2, TV_2, sizeof(struct timeval)), 0);
        tv3 = (struct timeval *) mmap(NULL, sizeof(struct timeval), PROT_READ | PROT_WRITE, MAP_SHARED,
                                      create_shm(&t3, TV_3, sizeof(struct timeval)), 0);
        mem = (char *) mmap(NULL, sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED,
                            create_shm(&m, SHM, sizeof(char)), 0);

        mutex_1 = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED,
                                           create_shm(&mutex, MUTEX_1, sizeof(pthread_mutex_t)), 0);
        cond_1 = (pthread_cond_t *) mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED,
                                         create_shm(&cond, CONDITION_1, sizeof(pthread_cond_t)), 0);

        pthread_mutexattr_t mattr;
        pthread_condattr_t cattr;

        pthread_mutexattr_init(&mattr);
        pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(mutex_1, &mattr);    /* Initialize mutex variable */

        pthread_condattr_init(&cattr);
        pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(cond_1, &cattr);  /* Initialize condition variable */

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
        printf("%f\n", (double)((tv3->tv_usec - tv2->tv_usec) - (tv2->tv_usec - tv1->tv_usec)));


        munmap(tv1, sizeof(struct timeval));
        munmap(tv2, sizeof(struct timeval));
        munmap(tv3, sizeof(struct timeval));
        shm_unlink(TV_1);
        shm_unlink(TV_2);
        shm_unlink(TV_3);

        munmap(mem, sizeof(char));
        shm_unlink(SHM);

        pthread_condattr_destroy(&cattr);
        pthread_mutexattr_destroy(&mattr);
        pthread_mutex_destroy(mutex_1);    /* Free up the_mutex */
        pthread_cond_destroy(cond_1);        /* Free up consumer condition variable */
        munmap(mutex_1, sizeof(pthread_mutex_t));
        munmap(cond_1, sizeof(pthread_cond_t));
        shm_unlink(MUTEX_1);
        shm_unlink(CONDITION_1);
    }
}

void start_timer() {
    gettimeofday(tv1, NULL);
    gettimeofday(tv2, NULL);
}

void stop_timer() {
    gettimeofday(tv3, NULL);
}

void *send_message() {
    pthread_mutex_lock(mutex_1);
    start_timer();
    memset(mem, 'r', sizeof(char));
    pthread_cond_signal(cond_1);
    pthread_mutex_unlock(mutex_1);
}

void *recv_message() {
    pthread_mutex_lock(mutex_1);
    while (*mem != 'r')
        pthread_cond_wait(cond_1, mutex_1);
//        printf(mem);
    stop_timer();
    pthread_mutex_unlock(mutex_1);
}