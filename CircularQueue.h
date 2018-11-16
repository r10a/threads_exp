//
// Created by rohit on 11/16/18.
//

#include <unistd.h>
#include <stdio.h>
#include "utils.h"

#ifndef THREADS_EXP_CIRCULARQUEUE_H
#define THREADS_EXP_CIRCULARQUEUE_H

#endif //THREADS_EXP_CIRCULARQUEUE_H

#define MAX_Q_SIZE 20

typedef struct Queue {
    int front, rear, size;
    request array[MAX_Q_SIZE];
} Queue;


void init(Queue *queue);

// Queue is full when size becomes equal to the capacity
int isFull(Queue *);

// Queue is empty when size is 0
int isEmpty(Queue *);

// Function to add an item to the queue.
int enqueue(Queue *, request *);

// Function to remove an item from queue.
// It changes front and size
int dequeue(Queue *, request *);

// Function to get front of queue
request *front(Queue *);

// Function to get rear of queue
request *rear(Queue *);