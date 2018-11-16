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
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <asm/errno.h>
#include <sys/syscall.h>

#define gettid() syscall(SYS_gettid)

//Own headers
//#include "ArrayQueue.h"
#include "CircularQueue.h"

#define N_NUM_THREADS 3
#define M_NUM_REQS 2

#define MUTEX_1 "/mutex_1"
#define MUTEX_2 "/mutex_2"
#define MUTEX_3 "/mutex_3"
#define MUTEX_4 "/mutex_4"

#define CONDITION_1 "/cond_1"
#define CONDITION_2 "/cond_2"
#define CONDITION_3 "/cond_3"
#define CONDITION_4 "/cond_4"

#define REQ_QUEUE_1 "/req_q_1"
#define RESP_QUEUE_1 "/resp_q_1"

#define REQ_QUEUE_2 "/req_q_2"
#define RESP_QUEUE_2 "/resp_q_2"

pthread_mutexattr_t mattr;
pthread_condattr_t cattr;

// REQUEST QUEUE 1
int shr_req_q_1, shr_cond_1, shr_mutex_1;
pthread_mutex_t *mutex_1;
pthread_cond_t *cond_1;
Queue *req_q_1;

// RESPONSE QUEUE 1
int shr_resp_q_1, shr_cond_2, shr_mutex_2;
pthread_mutex_t *mutex_2;
pthread_cond_t *cond_2;
Queue *resp_q_1;

// REQUEST QUEUE 2
int shr_req_q_2, shr_cond_3, shr_mutex_3;
pthread_mutex_t *mutex_3;
pthread_cond_t *cond_3;
Queue *req_q_2;

// RESPONSE QUEUE 2
int shr_resp_q_2, shr_cond_4, shr_mutex_4;
pthread_mutex_t *mutex_4;
pthread_cond_t *cond_4;
Queue *resp_q_2;


typedef struct thread_params {
    Queue *request_1;
    Queue *response_1;
    Queue *request_2;
    Queue *response_2;
} thread_params;

void *create_send_request(void *);

void *receive_send_response(void *);

void *forward_request_response(void *);

int share(int *share, char *path, int size);

void init_mutex_cond();

void cleanup();

int main(int argc, char **argv) {
    int pid_1, pid_2, pid_3, i;

    /* Map Mutex to shared memory */ /* Map Condition to shared memory */
    mutex_1 = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, share(&shr_mutex_1, MUTEX_1, sizeof(pthread_mutex_t)), 0);
    cond_1 = (pthread_cond_t *) mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED, share(&shr_cond_1, CONDITION_1, sizeof(pthread_cond_t)), 0);

    mutex_2 = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, share(&shr_mutex_2, MUTEX_2, sizeof(pthread_mutex_t)), 0);
    cond_2 = (pthread_cond_t *) mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED, share(&shr_cond_2, CONDITION_2, sizeof(pthread_cond_t)), 0);

    mutex_3 = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, share(&shr_mutex_3, MUTEX_3, sizeof(pthread_mutex_t)), 0);
    cond_3 = (pthread_cond_t *) mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED, share(&shr_cond_3, CONDITION_3, sizeof(pthread_cond_t)), 0);

    mutex_4 = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, share(&shr_mutex_4, MUTEX_4, sizeof(pthread_mutex_t)), 0);
    cond_4 = (pthread_cond_t *) mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED, share(&shr_cond_4, CONDITION_4, sizeof(pthread_cond_t)), 0);

    init_mutex_cond();

    share(&shr_req_q_1, REQ_QUEUE_1, sizeof(Queue));    /* Map queue to shared memory */
    req_q_1 = (Queue *) mmap(NULL, sizeof(Queue), PROT_READ | PROT_WRITE, MAP_SHARED, shr_req_q_1, 0);
    init(req_q_1);


    share(&shr_resp_q_1, RESP_QUEUE_1, sizeof(Queue));    /* Map queue to shared memory */
    resp_q_1 = (Queue *) mmap(NULL, sizeof(Queue), PROT_READ | PROT_WRITE, MAP_SHARED, shr_resp_q_1, 0);
    init(resp_q_1);

    share(&shr_req_q_2, REQ_QUEUE_2, sizeof(Queue));    /* Map queue to shared memory */
    req_q_2 = (Queue *) mmap(NULL, sizeof(Queue), PROT_READ | PROT_WRITE, MAP_SHARED, shr_req_q_2, 0);
    init(req_q_2);

    share(&shr_resp_q_2, RESP_QUEUE_2, sizeof(Queue));    /* Map queue to shared memory */
    resp_q_2 = (Queue *) mmap(NULL, sizeof(Queue), PROT_READ | PROT_WRITE, MAP_SHARED, shr_resp_q_2, 0);
    init(resp_q_2);


    thread_params *params_1 = malloc(sizeof(thread_params));   /* Create thread parameters */
    params_1->request_1 = req_q_1;
    params_1->response_1 = resp_q_1;

    thread_params *params_2 = malloc(sizeof(thread_params));   /* Create thread parameters */
    params_2->request_1 = req_q_1;
    params_2->response_1 = resp_q_1;
    params_2->request_2 = req_q_2;
    params_2->response_2 = resp_q_2;

    thread_params *params_3 = malloc(sizeof(thread_params));   /* Create thread parameters */
    params_3->request_1 = req_q_2;
    params_3->response_1 = resp_q_2;

    if ((pid_1 = fork()) == 0) { /* Child Process 1*/

        pthread_t threads[N_NUM_THREADS];
        print("\nprocess 1 created");

        for (i = 0; i < N_NUM_THREADS; i++) {
            pthread_create(&threads[i], NULL, create_send_request, params_1);
        }
        for (i = 0; i < N_NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }
        exit(0);
    } else if ((pid_2 = fork()) == 0) {  /* Child Process 2 */
        pthread_t threads[N_NUM_THREADS];
        print("\nprocess 2 created");

        for (i = 0; i < N_NUM_THREADS; i++) {
            pthread_create(&threads[i], NULL, forward_request_response, params_2);
        }
        for (i = 0; i < N_NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }
        exit(0);
    } else if ((pid_3 = fork()) == 0) {  /* Child Process 3 */
        pthread_t threads[N_NUM_THREADS];
        print("\nprocess 3 created");

        for (i = 0; i < N_NUM_THREADS; i++) {
            pthread_create(&threads[i], NULL, receive_send_response, params_3);
        }
        for (i = 0; i < N_NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }
        exit(0);
    }

    // Wait for all processes to finish
    int pid, status;
    while ((pid = wait(&status)) > 0);
    cleanup();
}


void *create_send_request(void *ptr) {

    // init
    int id = gettid();
    int i;
    request *req = malloc(sizeof(request));
    req->size = id;

    thread_params *params = (thread_params *) ptr;
    Queue *request = params->request_1;
    Queue *response = params->response_1;

    // sending request
    pthread_mutex_lock(mutex_1);
    for (i = 0; i < M_NUM_REQS; i++) {
        enqueue(request, req);
        print("\nSent request: ");
        print_int(req->size);
    }
    pthread_cond_signal(cond_1);
    pthread_mutex_unlock(mutex_1);

    // receiving response
    pthread_mutex_lock(mutex_4);
    while (isEmpty(response)) {
        pthread_cond_wait(cond_4, mutex_4);
    }
    for (i = 0; i < M_NUM_REQS; i++) {
        dequeue(response, req);
        print("\nReceived response: ");
        print_int(req->size);
    }
    pthread_mutex_unlock(mutex_4);
}

void *forward_request_response(void *ptr) {
    int i, o_id;
    request *req = malloc(sizeof(request));

    thread_params *params = (thread_params *) ptr;
    Queue *request_1 = params->request_1;
    Queue *response_1 = params->response_1;
    Queue *request_2 = params->request_2;
    Queue *response_2 = params->response_2;

    // receiving request
    pthread_mutex_lock(mutex_1);
    while (isEmpty(request_1)) {
        pthread_cond_wait(cond_1, mutex_1);
    }
    for (i = 0; i < M_NUM_REQS; i++) {
        dequeue(request_1, req);
        print("\nReceived request: ");
        print_int(req->size);
    }
    pthread_mutex_unlock(mutex_1);

    // forwarding request
    pthread_mutex_lock(mutex_2);
    for (i = 0; i < M_NUM_REQS; i++) {
        enqueue(request_2, req);
        print("\nForwarded request: ");
        print_int(req->size);
    }
    pthread_cond_signal(cond_2);
    pthread_mutex_unlock(mutex_2);

    // receiving response
    pthread_mutex_lock(mutex_3);
    while (isEmpty(response_2)) {
        pthread_cond_wait(cond_3, mutex_3);
    }
    for (i = 0; i < M_NUM_REQS; i++) {
        dequeue(response_2, req);
        print("\nReceived response: ");
        print_int(req->size);
    }
    pthread_mutex_unlock(mutex_3);

    // forwarding response
    pthread_mutex_lock(mutex_4);
    for (i = 0; i < M_NUM_REQS; i++) {
        enqueue(response_1, req);
        print("\nForwarded response: ");
        print_int(req->size);
    }
    pthread_cond_signal(cond_4);
    pthread_mutex_unlock(mutex_4);
}

void *receive_send_response(void *ptr) {

    // init
    int id = gettid();
    int i, o_id;
    request *req = malloc(sizeof(request));

    thread_params *params = (thread_params *) ptr;
    Queue *request = params->request_1;
    Queue *response = params->response_1;

    // receiving request
    pthread_mutex_lock(mutex_2);
    while (isEmpty(request)) {
        pthread_cond_wait(cond_2, mutex_2);
    }
    for (i = 0; i < M_NUM_REQS; i++) {
        dequeue(request, req);
        print("\nReceived request: ");
        print_int(req->size);
    }
    pthread_mutex_unlock(mutex_2);


    req->size = id;
    // sending response
    pthread_mutex_lock(mutex_3);
    for (i = 0; i < M_NUM_REQS; i++) {
        enqueue(response, req);
        print("\nSent response: ");
        print_int(req->size);
    }
    pthread_cond_signal(cond_3);
    pthread_mutex_unlock(mutex_3);
}

int share(int *share, char *path, int size) {
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

void init_mutex_cond() {
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex_1, &mattr);    /* Initialize mutex variable */
    pthread_mutex_init(mutex_2, &mattr);    /* Initialize mutex variable */
    pthread_mutex_init(mutex_3, &mattr);    /* Initialize mutex variable */
    pthread_mutex_init(mutex_4, &mattr);    /* Initialize mutex variable */
    pthread_mutexattr_destroy(&mattr);

    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(cond_1, &cattr);  /* Initialize condition variable */
    pthread_cond_init(cond_2, &cattr);  /* Initialize condition variable */
    pthread_cond_init(cond_3, &cattr);  /* Initialize condition variable */
    pthread_cond_init(cond_4, &cattr);  /* Initialize condition variable */
    pthread_condattr_destroy(&cattr);
}


void cleanup() {
    shm_unlink(MUTEX_1);
    shm_unlink(REQ_QUEUE_1);
    shm_unlink(CONDITION_1);
    shm_unlink(MUTEX_2);
    shm_unlink(RESP_QUEUE_1);
    shm_unlink(CONDITION_2);
    shm_unlink(MUTEX_3);
    shm_unlink(REQ_QUEUE_2);
    shm_unlink(CONDITION_3);
    shm_unlink(MUTEX_4);
    shm_unlink(RESP_QUEUE_2);
    shm_unlink(CONDITION_4);
    pthread_mutex_destroy(mutex_1);    /* Free up the_mutex */
    pthread_mutex_destroy(mutex_2);    /* Free up the_mutex */
    pthread_mutex_destroy(mutex_3);    /* Free up the_mutex */
    pthread_mutex_destroy(mutex_4);    /* Free up the_mutex */
    pthread_cond_destroy(cond_1);        /* Free up consumer condition variable */
    pthread_cond_destroy(cond_2);        /* Free up consumer condition variable */
    pthread_cond_destroy(cond_3);        /* Free up consumer condition variable */
    pthread_cond_destroy(cond_4);        /* Free up consumer condition variable */
}