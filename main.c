//
// Created by rohit on 10/26/18.
//
/*

*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define PROD_Q "/prod_q"
#define CONS_Q "/cons_q"
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE { MAX_MSG_SIZE + 10 }
#define QUEUE_PERMISSIONS 0660

pthread_mutex_t the_mutex;
pthread_mutexattr_t mattr;

pthread_cond_t condc, condp;
pthread_condattr_t cattr;

char in_buffer[MSG_BUFFER_SIZE];
char out_buffer[MSG_BUFFER_SIZE];
char print_buffer[MSG_BUFFER_SIZE];


void *producer(void *ptr) {

    struct mq_attr attr;
    mqd_t qd_prod;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    /*sprintf(print_buffer, "%p", &the_mutex);
    write(1, print_buffer, strlen(print_buffer));*/

//    sprintf(print_buffer, "%d", pthread_mutex_lock(&the_mutex));	/* protect buffer */
//    write(1, print_buffer, strlen(print_buffer));

    if ((qd_prod = mq_open(PROD_Q, O_WRONLY | O_NONBLOCK | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror("Prod: mq_open (prod)");
        exit(1);
    }

    sprintf(out_buffer, "%d", 11);

    while (1) {
        if (mq_send(qd_prod, out_buffer, strlen(out_buffer) + 1, 0) == -1) {
            perror("Prod: Not able to send message to client");
            continue;
        } else break;
    }
    mq_close(qd_prod);
    pthread_exit(0);
}

void *consumer(void *ptr) {

    mqd_t qd_cons;
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    /*sprintf(print_buffer, "%p", &the_mutex);
    write(1, print_buffer, strlen(print_buffer));*/

//    sprintf(print_buffer, "%d", pthread_mutex_lock(&the_mutex));	/* protect buffer */
//    write(1, print_buffer, strlen(print_buffer));

    if ((qd_cons = mq_open(PROD_Q, O_RDONLY | O_NONBLOCK | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror("Cons: mq_open (prod)");
        exit(1);
    }

    while (1) {
        if (mq_receive(qd_cons, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            perror("Cons: mq_receive");
        } else break;
    }
    write(1, "\nIn buffer: ", 12);
    write(1, in_buffer, strlen(in_buffer));
    mq_close(qd_cons);
    pthread_exit(0);
}

int main(int argc, char **argv) {
    pthread_t pro, con;
    int pid, pid_1, pid_2, status;

    // Initialize the mutex and condition variables
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&the_mutex, &mattr);
    pthread_mutexattr_destroy(&mattr);

    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&condc, &cattr);        /* Initialize consumer condition variable */
    pthread_cond_init(&condp, &cattr);        /* Initialize producer condition variable */
    pthread_condattr_destroy(&cattr);

//    printf("\nHello from parent process!");
    write(1, "\nParent", 7);
    /*sprintf(print_buffer, "%p", &the_mutex);
    write(1, print_buffer, strlen(print_buffer));*/


    //Create child processes
    if ((pid_1 = fork()) == 0) { /* Child process */

//        printf("\nHello from child process 1!");
        write(1, "\nprocess 1", 10);

        pthread_create(&pro, NULL, producer, NULL);
        pthread_join(pro, NULL);
        exit(0);

    } else { /* Create process 2 in parent */

        if ((pid_2 = fork()) == 0) { /* Child process */
//            printf("\nHello from child process 2!");
            write(1, "\nprocess 2", 10);
            pthread_create(&con, NULL, consumer, NULL);
            pthread_join(con, NULL);
            exit(0);
        }
    }
//    printf("%d", pid_1);


    // Create the threads
//    pthread_create(&con, NULL, consumer, NULL);
//    pthread_create(&pro, NULL, producer, NULL);

    // Wait for the threads to finish
    // Otherwise main might run to the end
    // and kill the entire process when it exits.
//    pthread_join(con, NULL);
//    pthread_join(pro, NULL);
    while ((pid = wait(&status)) > 0);
//    printf("\nIn Buffer is: %s", in_buffer);

    // Cleanup -- would happen automatically at end of program
    pthread_mutex_destroy(&the_mutex);    /* Free up the_mutex */
    pthread_cond_destroy(&condc);        /* Free up consumer condition variable */
    pthread_cond_destroy(&condp);        /* Free up producer condition variable */

}