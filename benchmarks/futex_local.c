// Same as futex-basic-process.c, but using threads.
//
// Main differences:
// 1. No need for shared memory calls (shmget, etc.), since threads share the
//    address space we can use a simple pointer.
// 2. Use the _PRIVATE versions of system calls since these can be more
//    efficient within a single process.
//
// Eli Bendersky [http://eli.thegreenplace.net]
// This code is in the public domain.
#include <errno.h>
#include <linux/futex.h>
#include <pthread.h>
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
#include "../utils.h"

struct timeval tv1, tv2, tv3;
int *shared_data;

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
        int futex_rc = futex(futex_addr, FUTEX_WAIT_PRIVATE, val, NULL, NULL, 0);
        if (futex_rc == -1) {
            if (errno != EAGAIN) {
                perror("futex");
                exit(1);
            }
        } else if (futex_rc == 0) {
            stop_timer();
            // This is a real wakeup.
            return;
        } else {
            abort();
        }
    }
}

// A blocking wrapper for waking a futex. Only returns when a waiter has been
// woken up.
void wake_futex_blocking(int *futex_addr) {
    while (1) {
        int futex_rc = futex(futex_addr, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
        if (futex_rc == -1) {
            perror("futex wake");
            exit(1);
        } else if (futex_rc > 0) {
            return;
        }
    }
}

int main() {
    for (int i = 0; i < 100; i++) {

        shared_data = (int *) malloc(sizeof(int));

        pthread_t thread1, thread2;
        pthread_create(&thread1, NULL, send_message, NULL);
        pthread_create(&thread2, NULL, recv_message, NULL);
        assign_thread_to_core(1, thread1);
        assign_thread_to_core(2, thread2);
        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);
        printf("%f\n", (double) ((tv3.tv_usec - tv2.tv_usec) - (tv2.tv_usec - tv1.tv_usec)));
    }
}

void start_timer() {
    gettimeofday(&tv1, NULL);
    gettimeofday(&tv2, NULL);
}

void stop_timer() {
    gettimeofday(&tv3, NULL);
}

void *send_message() {
    *shared_data = 20;
    start_timer();
    wake_futex_blocking(shared_data);
}

void *recv_message() {
    wait_on_futex_value(shared_data, 20);
}

