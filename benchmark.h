//
// Created by rohit on 1/26/2019.
//

#ifndef THREADS_EXP_BENCHMARK_H
#define THREADS_EXP_BENCHMARK_H

#ifndef size_lt
#define size_lt long long unsigned
#endif

static inline size_lt elapsed_time_ns(size_lt ns);

static inline size_lt elapsed_time_us(size_lt us);

static void* sender(void* params);

static void* intermediate(void *params);

static void* receiver(void *params);

int assign_thread_to_core(int core_id, pthread_t pthread);

#endif //THREADS_EXP_BENCHMARK_H
