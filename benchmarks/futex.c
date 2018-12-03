//
// Created by rohit on 12/2/2018.
//
#define _GNU_SOURCE

#include <errno.h>
#include <linux/futex.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include "../utils.h"

#include "futex.h"

#define TV_1 "/TIME1"
#define TV_2 "/TIME2"
#define TV_3 "/TIME3"

#define MSG "/msg"

struct timeval *tv1, *tv2, *tv3;
int *shared_data;
int futex_rc, i;

void start_timer();

void stop_timer();

void *send_message();

void *recv_message();

// The C runtime doesn't provide a wrapper for the futex(2) syscall, so we roll
// our own.
int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout,
          int *uaddr2, int val3) {
    return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}

// Waits for the futex at futex_addr to have the value val, ignoring spurious
// wakeups. This function only returns when the condition is fulfilled; the only
// other way out is aborting with an error.
void wait_on_futex_value(int *futex_addr, int val) {
    while (1) {
        futex_rc = futex(futex_addr, FUTEX_WAIT, val, NULL, NULL, 0);
        if (futex_rc == 0) {
            stop_timer();
            return;
        }
    }
}

// A blocking wrapper for waking a futex. Only returns when a waiter has been
// woken up.
void wake_futex_blocking(int *futex_addr) {
    while (1) {
        int futex_rc = futex(futex_addr, FUTEX_WAKE, 1, NULL, NULL, 0);
        if (futex_rc == -1) {
            perror("futex wake");
            exit(1);
        } else if (futex_rc > 0) {
            return;
        }
    }
}

int main(int argc, char **argv) {

    for (int i = 0; i < 100; i++) {

        int t1, t2, t3;
        int pid_1, pid_2;
        int m, assigned, actual;
        cpu_set_t cpuset1, cpuset2;
        CPU_ZERO(&cpuset1);
        CPU_ZERO(&cpuset2);
        CPU_SET(1, &cpuset1);
        CPU_SET(2, &cpuset2);


        tv1 = (struct timeval *) mmap(NULL, sizeof(struct timeval), PROT_READ | PROT_WRITE, MAP_SHARED,
                                      create_shm(&t1, TV_1, sizeof(struct timeval)), 0);
        tv2 = (struct timeval *) mmap(NULL, sizeof(struct timeval), PROT_READ | PROT_WRITE, MAP_SHARED,
                                      create_shm(&t2, TV_2, sizeof(struct timeval)), 0);
        tv3 = (struct timeval *) mmap(NULL, sizeof(struct timeval), PROT_READ | PROT_WRITE, MAP_SHARED,
                                      create_shm(&t3, TV_3, sizeof(struct timeval)), 0);
        shared_data = (int *) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED,
                                   create_shm(&m, MSG, sizeof(int)), 0);

        if ((pid_1 = fork()) == 0) {
            pthread_t thread;
            pthread_create(&thread, NULL, send_message, NULL);
            assign_thread_to_core(1, thread);
            actual = get_core_number(thread);
            pthread_join(thread, NULL);
            exit(0);
        } else {
            pthread_t thread;
            pthread_create(&thread, NULL, recv_message, NULL);
            assign_thread_to_core(2, thread);
            actual = get_core_number(thread);
            pthread_join(thread, NULL);
        }

        int pid, status;
        while ((pid = wait(&status)) > 0);
        printf("%f\n", (double) ((tv3->tv_usec - tv2->tv_usec) - (tv2->tv_usec - tv1->tv_usec)));

        munmap(tv1, sizeof(struct timeval));
        munmap(tv2, sizeof(struct timeval));
        munmap(tv3, sizeof(struct timeval));
        shm_unlink(TV_1);
        shm_unlink(TV_2);
        shm_unlink(TV_3);

        shm_unlink(MSG);
        munmap(shared_data, sizeof(int));
        usleep(1000);
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
    *shared_data = 20;
    start_timer();
    wake_futex_blocking(shared_data);
}

void *recv_message() {
    wait_on_futex_value(shared_data, 20);
}