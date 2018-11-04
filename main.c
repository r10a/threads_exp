//
// Created by rohit on 10/26/18.
//
/*

*/

// System headers
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <asm/errno.h>

//Own headers
#include "ArrayQueue.h"

#define N_NUM_THREADS 10
#define M_NUM_REQS 1

int shr_req_q_1, shr_cond_1, shr_mutex_1;
Queue *req_q_1, *resp_q_1;

pthread_mutex_t *mutex_1;
pthread_mutexattr_t mattr;

pthread_cond_t *cond_1;
pthread_condattr_t cattr;


typedef struct r_req {
    void* address;
    int size;
} r_req;

typedef struct w_req {
    void *address;
    int size;
} w_req;

void *create_send_request(void *);

int print(char *);

int print_int(int);

int share(int* share, char* path, int size);

void cleanup();

int main(int argc, char **argv) {
    int pid_1, pid_2, pid_3, i;

    Queue *req_q_1 = createQueue(M_NUM_REQS * N_NUM_THREADS);

    mutex_1 = (pthread_mutex_t*) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, share(&shr_mutex_1, "/mutex_1", sizeof(pthread_mutex_t)), 0);
    cond_1 = (pthread_cond_t*) mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED, share(&shr_cond_1, "/cond_1", sizeof(pthread_cond_t)), 0);

    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex_1, &mattr);
    pthread_mutexattr_destroy(&mattr);

    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(cond_1, &cattr);        /* Initialize condition variable */
    pthread_condattr_destroy(&cattr);

    share(&shr_req_q_1, "/req_q_1", sizeof(*req_q_1));
    req_q_1 = (Queue*) mmap(NULL, sizeof(*req_q_1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    req_q_1 = createQueue(M_NUM_REQS * N_NUM_THREADS);

    print_int(req_q_1->capacity);

    if((pid_1 = fork()) == 0) { /* Child Process*/

        pthread_t threads[N_NUM_THREADS];
        print("\nprocess 1 created");

        for(i = 0; i < N_NUM_THREADS; i++) {
            pthread_create(&threads[i], NULL, create_send_request, req_q_1);
        }
        for(i = 0; i < N_NUM_THREADS; i++){
            pthread_join(threads[i], NULL);
        }
        while(!isEmpty(req_q_1)) {
            print_int(dequeue(req_q_1));
        }
        exit(0);
    }

    cleanup();
}


void *create_send_request(void *ptr) {
    pthread_mutex_lock(mutex_1);
    Queue *queue = (Queue*) ptr;
    print("\ncreating request");
    enqueue(queue, pthread_self());
    pthread_mutex_unlock(mutex_1);
}

int print(char *str) {
    write(1, str, strlen(str));
}

int print_int(int num) {
    char number[20];
    sprintf(number, "\n%d", num);
    print(number);
}

int share(int* share, char* path, int size) {
    *share = shm_open(path, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG);

    if (*share < 0) {
        perror("failure on shm_open");
        exit(1);
    }
    if (ftruncate(*share, size) == -1) {
        perror("Error on ftruncate\n");
        exit(-1);
    }
    return *share;
}


void cleanup() {
    shm_unlink("/mutex_1");
    shm_unlink("/req_q_1");
    shm_unlink("/cond_1");
    pthread_mutex_destroy(mutex_1);    /* Free up the_mutex */
    pthread_cond_destroy(cond_1);        /* Free up consumer condition variable */
}