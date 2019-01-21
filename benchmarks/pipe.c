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
#include <stdint.h>
#include <time.h>
#include "../utils.h"

#include "pipe.h"

#define TV_1 "/TIME1"
#define TV_2 "/TIME2"
#define TV_3 "/TIME3"
#define TV_4 "/TIME4"

#define NUM_ITERS 1000000

#define size_lt unsigned long long

struct timeval *tv1, *tv2, *tv3;
int p[2];
char r;
char *msg = "r";

size_lt *timer;

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


void *send_message();

void *recv_message();

int main(int argc, char **argv) {

    int t1, t2, t3, t4;
    int pid_1, pid_2;


    tv1 = (struct timeval *) mmap(NULL, sizeof(struct timeval), PROT_READ | PROT_WRITE, MAP_SHARED,
                                  create_shm(&t1, TV_1, sizeof(struct timeval)), 0);
    tv2 = (struct timeval *) mmap(NULL, sizeof(struct timeval), PROT_READ | PROT_WRITE, MAP_SHARED,
                                  create_shm(&t2, TV_2, sizeof(struct timeval)), 0);
    tv3 = (struct timeval *) mmap(NULL, sizeof(struct timeval), PROT_READ | PROT_WRITE, MAP_SHARED,
                                  create_shm(&t3, TV_3, sizeof(struct timeval)), 0);
    timer = (size_lt *) mmap(NULL, sizeof(size_lt), PROT_READ | PROT_WRITE, MAP_SHARED,
                             create_shm(&t4, TV_4, sizeof(size_lt)), 0);


    if (pipe(p) < 0)
        perror("pipe");

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
//    printf("%f\n", (double) ((tv3->tv_usec - tv2->tv_usec) - (tv2->tv_usec - tv1->tv_usec)));
//    printf("%f\n", (double)*timer / NUM_ITERS);

    munmap(tv1, sizeof(struct timeval));
    munmap(tv2, sizeof(struct timeval));
    munmap(tv3, sizeof(struct timeval));
    munmap(timer, sizeof(size_lt));
    shm_unlink(TV_1);
    shm_unlink(TV_2);
    shm_unlink(TV_3);
    shm_unlink(TV_4);
}

static inline void start_timer() {
    *timer = elapsed_time(0);
}

static inline void stop_timer() {
    *timer = elapsed_time(*timer);
    printf("%llu\n", *timer/NUM_ITERS);
}


void *send_message() {
    close(p[0]);
    start_timer();
    for (int i = 0; i < NUM_ITERS; i++) {
        write(p[1], msg, sizeof(char));
    }
    close(p[1]);
}

void *recv_message() {
    close(p[1]);
    for (int i = 0; i < NUM_ITERS; i++) {
        read(p[0], &r, sizeof(char));
    }
    stop_timer();
    close(p[0]);
}