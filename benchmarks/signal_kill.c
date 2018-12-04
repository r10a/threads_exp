#define _XOPEN_SOURCE 700

#include <assert.h>
#include <signal.h>
#include <stdbool.h> /* false */
#include <stdio.h> /* perror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <sys/wait.h> /* wait, sleep */
#include <unistd.h> /* fork, write */
#include <sys/time.h>
#include <sys/mman.h>
#include "signal_kill.h"
#include "../utils.h"

#define TV_1 "/TIME1"
#define TV_2 "/TIME2"
#define TV_3 "/TIME3"

#define MSG "/msg"

struct timeval *tv1, *tv2, *tv3;
int *shared_data;

void start_timer();

void stop_timer();

void *send_message();

void *recv_message();

void signal_handler(int sig);

int main() {
//    for (int i = 0; i < 100; i++) {
    int t1, t2, t3;
    int pid_1, pid_2;
    int m;

    tv1 = (struct timeval *) mmap(NULL, sizeof(struct timeval), PROT_READ | PROT_WRITE, MAP_SHARED,
                                  create_shm(&t1, TV_1, sizeof(struct timeval)), 0);
    tv2 = (struct timeval *) mmap(NULL, sizeof(struct timeval), PROT_READ | PROT_WRITE, MAP_SHARED,
                                  create_shm(&t2, TV_2, sizeof(struct timeval)), 0);
    tv3 = (struct timeval *) mmap(NULL, sizeof(struct timeval), PROT_READ | PROT_WRITE, MAP_SHARED,
                                  create_shm(&t3, TV_3, sizeof(struct timeval)), 0);
    shared_data = (int *) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED,
                               create_shm(&m, MSG, sizeof(int)), 0);

    signal(SIGUSR1, signal_handler);

    if ((pid_1 = fork()) == 0) {
        sleep(1);
    } else {
        *shared_data = 20;
//            start_timer();
        gettimeofday(tv2, NULL);
        print_int(tv2->tv_usec);
        kill(pid_1, SIGUSR1);
    }
    printf("%f\n", (double) ((tv3->tv_usec - tv2->tv_usec) - (tv2->tv_usec - tv1->tv_usec)));

    munmap(tv1, sizeof(struct timeval));
    munmap(tv2, sizeof(struct timeval));
    munmap(tv3, sizeof(struct timeval));
    shm_unlink(TV_1);
    shm_unlink(TV_2);
    shm_unlink(TV_3);

    shm_unlink(MSG);
    munmap(shared_data, sizeof(int));
//    usleep(1000);
//    }
}

void signal_handler(int sig) {
    char s1[] = "SIGUSR1\n";
    if (sig == SIGUSR1) {
        gettimeofday(tv3, NULL);
        print_int(tv3->tv_usec);
//        stop_timer();
        write(STDOUT_FILENO, s1, sizeof(s1));
        print("\nDone");
        print_int(*shared_data);
        exit(EXIT_SUCCESS);
    }
//    signal(sig, signal_handler);
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
}

void *recv_message() {
//    wait_on_futex_value(shared_data, 20);
}