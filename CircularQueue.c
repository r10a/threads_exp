//
// Created by rohit on 11/16/18.
//

#include <stddef.h>
#include <memory.h>
#include "CircularQueue.h"


void init(Queue *queue) {
    queue->front = queue->rear = -1;
    queue->size = MAX_Q_SIZE;
}

void copy_request(request *dest, request *src) {
    memcpy(dest, src, sizeof(request));
}

int enqueue(Queue *queue, request *req) {

    if (isFull(queue)) {
        return -1;
    } else if (queue->front == -1) { /* Insert First Element */
        queue->front = queue->rear = 0;
    } else if (queue->rear == queue->size - 1 && queue->front != 0) { /* Rear is at end of pool */
        queue->rear = 0;
    } else {
        queue->rear++;
    }
    copy_request(&queue->array[queue->rear], req);
    return 0;
}

int dequeue(Queue *queue, request *req) {
    if (isEmpty(queue)) {
        return -1;
    }

    copy_request(req, &queue->array[queue->front]);

    // TODO: Set queue location empty?
    memset(&queue->array[queue->front], 0, sizeof(request));

    if (queue->front == queue->rear) {
        queue->front = -1;
        queue->rear = -1;
    } else if (queue->front == queue->size - 1)
        queue->front = 0;
    else
        queue->front++;
    return 0;
}

int isEmpty(Queue *queue) {
    return  queue->front == -1;
}

int isFull(Queue *queue) {
    return (queue->front == 0 && queue->rear == queue->size - 1) || (queue->rear == (queue->front - 1) % (queue->size - 1));
}

request *front(Queue *queue) {
    if (isEmpty(queue)) {
        return NULL;
    }
    return &queue->array[queue->front];
}

request *rear(Queue *queue) {
    if (isEmpty(queue)) {
        return NULL;
    }
    return &queue->array[queue->rear];
}