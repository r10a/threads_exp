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

#define N_NUM_THREADS 1
#define M_NUM_REQS 1

int shr_req_q_1, shr_cond_1, shr_mutex_1;
int shr_resp_q_1, shr_cond_2, shr_mutex_2;
Queue *req_q_1, *resp_q_1;

pthread_mutex_t *mutex_1, *mutex_2;
pthread_mutexattr_t mattr;

pthread_cond_t *cond_1, *cond_2;
pthread_condattr_t cattr;

typedef struct thread_params {
    Queue* request;
    Queue* response;
} thread_params;


typedef struct r_req {
    void* address;
    int size;
} r_req;

typedef struct w_req {
    void *address;
    int size;
} w_req;

void *create_send_request(void *);

void *receive_send_response(void *);

int print(char *);

int print_int(int);

int share(int* share, char* path, int size);

void cleanup();

int main(int argc, char **argv) {
    int pid_1, pid_2, pid_3, i;

    req_q_1 = createQueue(M_NUM_REQS * N_NUM_THREADS);

    mutex_1 = (pthread_mutex_t*) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, share(&shr_mutex_1, "/mutex_1", sizeof(pthread_mutex_t)), 0);
    cond_1 = (pthread_cond_t*) mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED, share(&shr_cond_1, "/cond_1", sizeof(pthread_cond_t)), 0);

    mutex_2 = (pthread_mutex_t*) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, share(&shr_mutex_2, "/mutex_2", sizeof(pthread_mutex_t)), 0);
    cond_2 = (pthread_cond_t*) mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED, share(&shr_cond_2, "/cond_2", sizeof(pthread_cond_t)), 0);

    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex_1, &mattr);
    pthread_mutex_init(mutex_2, &mattr);
    pthread_mutexattr_destroy(&mattr);

    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(cond_1, &cattr);        /* Initialize condition variable */
    pthread_cond_init(cond_2, &cattr);        /* Initialize condition variable */
    pthread_condattr_destroy(&cattr);

    share(&shr_req_q_1, "/req_q_1", sizeof(*req_q_1));
    req_q_1 = (Queue*) mmap(NULL, sizeof(*req_q_1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    req_q_1 = createQueue(M_NUM_REQS * N_NUM_THREADS);

    share(&shr_resp_q_1, "/resp_q_1", sizeof(*req_q_1));
    resp_q_1 = (Queue*) mmap(NULL, sizeof(*req_q_1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    resp_q_1 = createQueue(M_NUM_REQS * N_NUM_THREADS);

//    print_int(req_q_1->capacity);

    if((pid_1 = fork()) == 0) { /* Child Process 1*/

        pthread_t threads[N_NUM_THREADS];
        print("\nprocess 1 created");

        thread_params *params = malloc(sizeof(thread_params));
        params->request = req_q_1;
        params->response = resp_q_1;

        for(i = 0; i < N_NUM_THREADS; i++) {
            pthread_create(&threads[i], NULL, create_send_request, params);
        }
        for(i = 0; i < N_NUM_THREADS; i++){
            pthread_join(threads[i], NULL);
        }
        while(!isEmpty(resp_q_1)) {
            print_int(dequeue(resp_q_1));
        }
        exit(0);
    } else if((pid_2 = fork()) == 0) {  /*Child Process 2*/
        pthread_t threads[N_NUM_THREADS];
        print("\nprocess 2 created");

        thread_params *params = malloc(sizeof(thread_params));
        params->request = req_q_1;
        params->response = resp_q_1;

        for(i = 0; i < N_NUM_THREADS; i++) {
            pthread_create(&threads[i], NULL, receive_send_response, params);
        }
        for(i = 0; i < N_NUM_THREADS; i++) {
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
    int id = pthread_self();
    thread_params *params = (thread_params*) ptr;
    Queue *request = params->request;
    Queue *response = params->response;

    // sending request
    pthread_mutex_lock(mutex_1);
    print("\ncreating request");
    print_int(id);
    enqueue(request, id);
    print("\nenqueued");
    pthread_cond_signal(cond_1);
    print("\nsignaled");
    pthread_mutex_unlock(mutex_1);
    print("\nunlocked");

//    print("\nrequest sent");
    // receiving response
    /*pthread_mutex_lock(mutex_2);
    while(isEmpty(response)) {
//        print("\nwaiting for response");
        pthread_cond_wait(cond_2, mutex_2);
    }
    print("\nreceiving request process 1");
    int o_id = dequeue(response);
    print_int(o_id);
    pthread_mutex_unlock(mutex_2);*/
}

void *receive_send_response(void *ptr) {

    // init
    int id = pthread_self();
    thread_params *params = (thread_params*) ptr;
    Queue *request = params->request;
    Queue *response = params->response;

    // receiving request
    print("\nreceiving request");
    pthread_mutex_lock(mutex_1);
    print("\nlocked");
    while(isEmpty(request)) {
        print("\nunlocked & waiting");
        print_int(isEmpty(request));
        print_int(isFull(request));
        pthread_cond_wait(cond_1, mutex_1);
    }
    //print("\nWaited successfully");
    int o_id = dequeue(request);
    print("\nreceiving request process 2");
    print_int(o_id);
    pthread_mutex_unlock(mutex_1);

    // sending response
    /*pthread_mutex_lock(mutex_2);
    print("\nsending response");
    enqueue(response, id);
    pthread_cond_signal(cond_2);
    pthread_mutex_unlock(mutex_2);*/
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
    shm_unlink("/mutex_2");
    shm_unlink("/resp_q_1");
    shm_unlink("/cond_2");
    pthread_mutex_destroy(mutex_1);    /* Free up the_mutex */
    pthread_mutex_destroy(mutex_2);    /* Free up the_mutex */
    pthread_cond_destroy(cond_1);        /* Free up consumer condition variable */
    pthread_cond_destroy(cond_2);        /* Free up consumer condition variable */
}