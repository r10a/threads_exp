//
// Created by rohit on 1/25/2019.
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
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include "../utils.h"
#include "sockets.h"
//
// Created by rohit on 12/2/2018.
//
#define TV_1 "/TIME1"
#define TV_2 "/TIME2"

#define NUM_ITERS 1000000

#define size_lt unsigned long long

char *socket_path = "\0hidden";

size_lt results[NUM_ITERS];
size_lt *timer_us, *timer_ns;

void *send_message();

void *recv_message();

static int sv[2]; /* the pair of socket descriptors */
static char buf; /* for data exchange between processes */

int main(int argc, char **argv) {

    int t1, t2;

    timer_us = (size_lt *) mmap(NULL, sizeof(size_lt), PROT_READ | PROT_WRITE, MAP_SHARED,
                                create_shm(&t1, TV_1, sizeof(size_lt)), 0);
    timer_ns = (size_lt *) mmap(NULL, sizeof(size_lt), PROT_READ | PROT_WRITE, MAP_SHARED,
                                create_shm(&t2, TV_2, sizeof(size_lt)), 0);


    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
//    if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0, sv) == -1) {
//    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) == -1) {
        perror("socketpair");
        exit(1);
    }

    if (fork() == 0) {
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
    size_lt sum;
    for (int i = 0; i < NUM_ITERS; i++) {
        sum += results[i];
    }
    printf("Total: %llu | Mean: %f", sum, ((double) sum) / NUM_ITERS);

    munmap(timer_us, sizeof(size_lt));
    munmap(timer_ns, sizeof(size_lt));
    shm_unlink(TV_1);
    shm_unlink(TV_2);
    return 0;
}

static inline void start_timer() {
    *timer_ns = elapsed_time_ns(0);
    *timer_us = elapsed_time_us(0);
}

static inline size_lt stop_timer() {
//    *timer_ns = elapsed_time_ns(*timer_ns);
//    *timer_us = elapsed_time_us(*timer_us);
//    printf("Time taken: %llu us | Per iter: %f us\n", *timer_us, ((double)*timer_us)/NUM_ITERS);
//    printf("Time taken: %llu ns | Per iter: %f ns\n", *timer_ns, ((double)*timer_ns)/NUM_ITERS);
    return elapsed_time_ns(*timer_ns);
}

void *send_message() {
    start_timer();
    for (int i = 0; i < NUM_ITERS; i++) {
        buf = (char)i;
        write(sv[1], &buf, 1);
    }
    printf("child: sent '%c'\n", buf);
    return 0;
}

void *recv_message() {
    for (int i = 0; i < NUM_ITERS; i++) {
        read(sv[0], &buf, 1);
    }
    printf("\nTotal: %llu | Mean: %f", stop_timer(), ((double) stop_timer()) / NUM_ITERS);
    printf("\nparent: read '%d'\n", buf);
    return 0;
}
