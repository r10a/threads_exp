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

#include "pipe.h"

#define TV_1 "/TIME1"
#define TV_2 "/TIME2"
#define TV_3 "/TIME3"

struct timeval *tv1, *tv2, *tv3;
int p[2];
char r;
char *msg = "r";

void start_timer();

void stop_timer();

void *send_message();

void *recv_message();

int main(int argc, char **argv) {

    for (int i = 0; i < 100; i++) {


        int t1, t2, t3;
        int pid_1, pid_2;


        tv1 = (struct timeval *) mmap(NULL, sizeof(struct timeval), PROT_READ | PROT_WRITE, MAP_SHARED,
                                      create_shm(&t1, TV_1, sizeof(struct timeval)), 0);
        tv2 = (struct timeval *) mmap(NULL, sizeof(struct timeval), PROT_READ | PROT_WRITE, MAP_SHARED,
                                      create_shm(&t2, TV_2, sizeof(struct timeval)), 0);
        tv3 = (struct timeval *) mmap(NULL, sizeof(struct timeval), PROT_READ | PROT_WRITE, MAP_SHARED,
                                      create_shm(&t3, TV_3, sizeof(struct timeval)), 0);


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
        printf("%f\n", (double) ((tv3->tv_usec - tv2->tv_usec) - (tv2->tv_usec - tv1->tv_usec)));

        munmap(tv1, sizeof(struct timeval));
        munmap(tv2, sizeof(struct timeval));
        munmap(tv3, sizeof(struct timeval));
        shm_unlink(TV_1);
        shm_unlink(TV_2);
        shm_unlink(TV_3);
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
    close(p[0]);
    start_timer();
    write(p[1], msg, sizeof(char));
    close(p[1]);
}

void *recv_message() {
    close(p[1]);
    read(p[0], &r, sizeof(char));
    stop_timer();
    close(p[0]);
}