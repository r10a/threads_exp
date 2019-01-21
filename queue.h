//
// Created by rohit on 1/19/2019.
//
#ifndef THREADS_EXP_QUEUE_H
#define THREADS_EXP_QUEUE_H
#define ORIGINAL
#ifndef ORIGINAL
void init_queue(int nprocs);

void thread_init(int id);

void thread_exit(int id);

static inline void wfenqueue(int id, void* val);

static inline void* wfdequeue(int id)

#else
void init_queue(int nprocs, int logn);
void thread_init(int id, int nprocs);
void * benchmark(int id, int nprocs);
void thread_exit(int id, int nprocs);
int verify(int nprocs, void ** results);
#endif
#endif //THREADS_EXP_QUEUE_H
