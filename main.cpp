#include <iostream>
#include <iterator>
#include <algorithm>
#include <unistd.h>
#include <climits>
#include <sys/wait.h>
#include "MSQueue.hpp"

#ifndef NUM_ITERS
#define NUM_ITERS 1000000
#endif

#ifndef NUM_RUNS
#define NUM_RUNS 3
#endif

#define NUM_THREAD 1

#define size_lt unsigned long long
#define SHM_G "GLOBAL"
#define SHM_Q1 "NODE_POOL_1"
#define SHM_Q2 "NODE_POOL_2"
#define SHM_Q3 "NODE_POOL_3"
#define SHM_Q4 "NODE_POOL_4"
#define SHM_Q5 "NODE_POOL_6"
#define SHM_Q6 "NODE_POOL_6"
#define SHM_Q7 "NODE_POOL_7"
#define SHM_Q8 "NODE_POOL_8"

#define SHM_Q9 "NODE_POOL_9"
#define SHM_Q10 "NODE_POOL_10"
#define SHM_Q11 "NODE_POOL_11"
#define SHM_Q12 "NODE_POOL_12"
#define SHM_Q13 "NODE_POOL_13"
#define SHM_Q14 "NODE_POOL_14"
#define SHM_Q15 "NODE_POOL_15"
#define SHM_Q16 "NODE_POOL_16"

#define SHM_Q17 "NODE_POOL_17"
#define SHM_Q18 "NODE_POOL_18"
#define SHM_Q19 "NODE_POOL_19"
#define SHM_Q20 "NODE_POOL_20"
#define SHM_Q21 "NODE_POOL_21"
#define SHM_Q22 "NODE_POOL_22"
#define SHM_Q23 "NODE_POOL_23"
#define SHM_Q24 "NODE_POOL_24"

#define SHM_Q25 "NODE_POOL_25"
#define SHM_Q26 "NODE_POOL_26"
#define SHM_Q27 "NODE_POOL_27"
#define SHM_Q28 "NODE_POOL_28"
#define SHM_Q29 "NODE_POOL_29"
#define SHM_Q30 "NODE_POOL_30"
#define SHM_Q31 "NODE_POOL_31"
#define SHM_Q32 "NODE_POOL_32"


pthread_barrier_t *barrier_t;
size_lt *reqs;

static void *sender(void *params);

static void *intermediate(void *params);

static void *receiver(void *params);

static inline size_lt elapsed_time_ns(size_lt ns) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000000000L + t.tv_nsec - ns;
}

int assign_thread_to_core(int core_id, pthread_t pthread) {
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (core_id < 0 || core_id >= num_cores)
        return EINVAL;

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    return pthread_setaffinity_np(pthread, sizeof(cpu_set_t), &cpuset);
}

typedef struct params {
    int id;
    MSQueue<size_lt> *q12;
    MSQueue<size_lt> *q23;
    MSQueue<size_lt> *q32;
    MSQueue<size_lt> *q21;
    size_lt buf;
} params;


void shm_cleanup() {
    shared_memory_object::remove(SHM_G);
    shared_memory_object::remove(SHM_Q1);
    shared_memory_object::remove(SHM_Q2);
    shared_memory_object::remove(SHM_Q3);
    shared_memory_object::remove(SHM_Q4);
    shared_memory_object::remove(SHM_Q5);
    shared_memory_object::remove(SHM_Q6);
    shared_memory_object::remove(SHM_Q7);
    shared_memory_object::remove(SHM_Q8);
    shared_memory_object::remove(SHM_Q9);
    shared_memory_object::remove(SHM_Q10);
    shared_memory_object::remove(SHM_Q11);
    shared_memory_object::remove(SHM_Q12);
    shared_memory_object::remove(SHM_Q13);
    shared_memory_object::remove(SHM_Q14);
    shared_memory_object::remove(SHM_Q15);
    shared_memory_object::remove(SHM_Q16);
    shared_memory_object::remove(SHM_Q17);
    shared_memory_object::remove(SHM_Q18);
    shared_memory_object::remove(SHM_Q19);
    shared_memory_object::remove(SHM_Q20);
    shared_memory_object::remove(SHM_Q21);
    shared_memory_object::remove(SHM_Q22);
    shared_memory_object::remove(SHM_Q23);
    shared_memory_object::remove(SHM_Q24);
    shared_memory_object::remove(SHM_Q25);
    shared_memory_object::remove(SHM_Q26);
    shared_memory_object::remove(SHM_Q27);
    shared_memory_object::remove(SHM_Q28);
    shared_memory_object::remove(SHM_Q29);
    shared_memory_object::remove(SHM_Q30);
    shared_memory_object::remove(SHM_Q31);
    shared_memory_object::remove(SHM_Q32);

    struct shm_remove {
        shm_remove() {
            shared_memory_object::remove(SHM_G);
            shared_memory_object::remove(SHM_Q1);
            shared_memory_object::remove(SHM_Q2);
            shared_memory_object::remove(SHM_Q3);
            shared_memory_object::remove(SHM_Q4);
            shared_memory_object::remove(SHM_Q5);
            shared_memory_object::remove(SHM_Q6);
            shared_memory_object::remove(SHM_Q7);
            shared_memory_object::remove(SHM_Q8);
            shared_memory_object::remove(SHM_Q9);
            shared_memory_object::remove(SHM_Q10);
            shared_memory_object::remove(SHM_Q11);
            shared_memory_object::remove(SHM_Q12);
            shared_memory_object::remove(SHM_Q13);
            shared_memory_object::remove(SHM_Q14);
            shared_memory_object::remove(SHM_Q15);
            shared_memory_object::remove(SHM_Q16);
            shared_memory_object::remove(SHM_Q17);
            shared_memory_object::remove(SHM_Q18);
            shared_memory_object::remove(SHM_Q19);
            shared_memory_object::remove(SHM_Q20);
            shared_memory_object::remove(SHM_Q21);
            shared_memory_object::remove(SHM_Q22);
            shared_memory_object::remove(SHM_Q23);
            shared_memory_object::remove(SHM_Q24);
            shared_memory_object::remove(SHM_Q25);
            shared_memory_object::remove(SHM_Q26);
            shared_memory_object::remove(SHM_Q27);
            shared_memory_object::remove(SHM_Q28);
            shared_memory_object::remove(SHM_Q29);
            shared_memory_object::remove(SHM_Q30);
            shared_memory_object::remove(SHM_Q31);
            shared_memory_object::remove(SHM_Q32);
        }

        ~shm_remove() {
            shared_memory_object::remove(SHM_G);
            shared_memory_object::remove(SHM_Q1);
            shared_memory_object::remove(SHM_Q2);
            shared_memory_object::remove(SHM_Q3);
            shared_memory_object::remove(SHM_Q4);
            shared_memory_object::remove(SHM_Q5);
            shared_memory_object::remove(SHM_Q6);
            shared_memory_object::remove(SHM_Q7);
            shared_memory_object::remove(SHM_Q8);
            shared_memory_object::remove(SHM_Q9);
            shared_memory_object::remove(SHM_Q10);
            shared_memory_object::remove(SHM_Q11);
            shared_memory_object::remove(SHM_Q12);
            shared_memory_object::remove(SHM_Q13);
            shared_memory_object::remove(SHM_Q14);
            shared_memory_object::remove(SHM_Q15);
            shared_memory_object::remove(SHM_Q16);
            shared_memory_object::remove(SHM_Q17);
            shared_memory_object::remove(SHM_Q18);
            shared_memory_object::remove(SHM_Q19);
            shared_memory_object::remove(SHM_Q20);
            shared_memory_object::remove(SHM_Q21);
            shared_memory_object::remove(SHM_Q22);
            shared_memory_object::remove(SHM_Q23);
            shared_memory_object::remove(SHM_Q24);
            shared_memory_object::remove(SHM_Q25);
            shared_memory_object::remove(SHM_Q26);
            shared_memory_object::remove(SHM_Q27);
            shared_memory_object::remove(SHM_Q28);
            shared_memory_object::remove(SHM_Q29);
            shared_memory_object::remove(SHM_Q30);
            shared_memory_object::remove(SHM_Q31);
            shared_memory_object::remove(SHM_Q32);
        }
    } remover;
}

int main() {
    shm_cleanup();
    size_t size = 50000000; // 50 MB

    fixed_managed_shared_memory managed_shm(open_or_create, SHM_G, size, (void *) 0x1000000000L);

    barrier_t = managed_shm.construct<pthread_barrier_t>(anonymous_instance)();
    pthread_barrierattr_t barattr;
    pthread_barrierattr_setpshared(&barattr, PTHREAD_PROCESS_SHARED);
    pthread_barrier_init(barrier_t, &barattr, NUM_THREAD * 3);
    pthread_barrierattr_destroy(&barattr);

    /*fixed_managed_shared_memory shm1(create_only, SHM_Q1, size);
    fixed_managed_shared_memory shm2(create_only, SHM_Q2, size);
    fixed_managed_shared_memory shm3(create_only, SHM_Q3, size);
    fixed_managed_shared_memory shm4(create_only, SHM_Q4, size);

    MSQueue<size_lt> *q12 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm1,
                                                                                        NUM_THREAD * 2);
    MSQueue<size_lt> *q23 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm2,
                                                                                        NUM_THREAD * 2);
    MSQueue<size_lt> *q32 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm3,
                                                                                        NUM_THREAD * 2);
    MSQueue<size_lt> *q21 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm4,
                                                                                        NUM_THREAD * 2);
    params p[NUM_THREAD];
    for (int i = 0; i < NUM_THREAD; i++) {
        p[i].id = i;
        p[i].q12 = q12;
        p[i].q23 = q23;
        p[i].q32 = q32;
        p[i].q21 = q21;
    }
*/
    fixed_managed_shared_memory shm1(open_or_create, SHM_Q1, size);
    fixed_managed_shared_memory shm2(open_or_create, SHM_Q2, size);
    fixed_managed_shared_memory shm3(open_or_create, SHM_Q3, size);
    fixed_managed_shared_memory shm4(open_or_create, SHM_Q4, size);
    fixed_managed_shared_memory shm5(open_or_create, SHM_Q5, size);
    fixed_managed_shared_memory shm6(open_or_create, SHM_Q6, size);
    fixed_managed_shared_memory shm7(open_or_create, SHM_Q7, size);
    fixed_managed_shared_memory shm8(open_or_create, SHM_Q8, size);
    fixed_managed_shared_memory shm9(open_or_create, SHM_Q9, size);
    fixed_managed_shared_memory shm10(open_or_create, SHM_Q10, size);
    fixed_managed_shared_memory shm11(open_or_create, SHM_Q11, size);
    fixed_managed_shared_memory shm12(open_or_create, SHM_Q12, size);
    fixed_managed_shared_memory shm13(open_or_create, SHM_Q13, size);
    fixed_managed_shared_memory shm14(open_or_create, SHM_Q14, size);
    fixed_managed_shared_memory shm15(open_or_create, SHM_Q15, size);
    fixed_managed_shared_memory shm16(open_or_create, SHM_Q16, size);
    fixed_managed_shared_memory shm17(open_or_create, SHM_Q17, size);
    fixed_managed_shared_memory shm18(open_or_create, SHM_Q18, size);
    fixed_managed_shared_memory shm19(open_or_create, SHM_Q19, size);
    fixed_managed_shared_memory shm20(open_or_create, SHM_Q20, size);
    fixed_managed_shared_memory shm21(open_or_create, SHM_Q21, size);
    fixed_managed_shared_memory shm22(open_or_create, SHM_Q22, size);
    fixed_managed_shared_memory shm23(open_or_create, SHM_Q23, size);
    fixed_managed_shared_memory shm24(open_or_create, SHM_Q24, size);
    fixed_managed_shared_memory shm25(open_or_create, SHM_Q25, size);
    fixed_managed_shared_memory shm26(open_or_create, SHM_Q26, size);
    fixed_managed_shared_memory shm27(open_or_create, SHM_Q27, size);
    fixed_managed_shared_memory shm28(open_or_create, SHM_Q28, size);
    fixed_managed_shared_memory shm29(open_or_create, SHM_Q29, size);
    fixed_managed_shared_memory shm30(open_or_create, SHM_Q30, size);
    fixed_managed_shared_memory shm31(open_or_create, SHM_Q31, size);
    fixed_managed_shared_memory shm32(open_or_create, SHM_Q32, size);

    params p[8];
    p[0].id = 0;
    p[0].q12 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm1, 2);
    p[0].q23 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm2, 2);
    p[0].q32 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm3, 2);
    p[0].q21 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm4, 2);
    p[0].buf = 0;

    p[1].id = 1;
    p[1].q12 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm5, 2);
    p[1].q23 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm6, 2);
    p[1].q32 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm7, 2);
    p[1].q21 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm8, 2);
    p[1].buf = 0;

    p[2].id = 2;
    p[2].q12 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm9, 2);
    p[2].q23 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm10, 2);
    p[2].q32 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm11, 2);
    p[2].q21 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm12, 2);
    p[2].buf = 0;

    p[3].id = 3;
    p[3].q12 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm13, 2);
    p[3].q23 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm14, 2);
    p[3].q32 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm15, 2);
    p[3].q21 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm16, 2);
    p[3].buf = 0;

    p[4].id = 4;
    p[4].q12 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm17, 2);
    p[4].q23 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm18, 2);
    p[4].q32 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm19, 2);
    p[4].q21 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm20, 2);
    p[4].buf = 0;

    p[5].id = 5;
    p[5].q12 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm21, 2);
    p[5].q23 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm22, 2);
    p[5].q32 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm23, 2);
    p[5].q21 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm24, 2);
    p[5].buf = 0;

    p[6].id = 6;
    p[6].q12 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm25, 2);
    p[6].q23 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm26, 2);
    p[6].q32 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm27, 2);
    p[6].q21 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm28, 2);
    p[6].buf = 0;

    p[7].id = 7;
    p[7].q12 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm29, 2);
    p[7].q23 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm30, 2);
    p[7].q32 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm31, 2);
    p[7].q21 = managed_shm.construct<MSQueue<size_lt>>(anonymous_instance)(&shm32, 2);
    p[7].buf = 0;

    reqs = managed_shm.construct<size_lt>(anonymous_instance)[NUM_ITERS]();
    for (int i = 0; i < NUM_ITERS; i++) reqs[i] = i;

    if (fork() == 0) {
        pthread_t s_threads[NUM_THREAD];
        for (int i = 0; i < NUM_THREAD; i++) {
            pthread_create(&s_threads[i], nullptr, sender, &p[i]);
            assign_thread_to_core(i, s_threads[i]);
        }

        for (unsigned long s_thread : s_threads) {
            pthread_join(s_thread, nullptr);
        }
        size_lt avg = 0, min = INT_MAX, max = 0;
        for (auto &i : p) {
            avg += i.buf;
            min = min > i.buf ? i.buf : min;
            max = max > i.buf ? max : i.buf;
        }
        printf("Avg time taken: %f ns | Max: %llu | Min: %llu\n", ((double) avg) / NUM_THREAD, max, min);
        printf("Process 1 completed\n");
        exit(0);
    }

    if (fork() == 0) {
        pthread_t i_threads[NUM_THREAD];
        for (int i = 0; i < NUM_THREAD; i++) {
            pthread_create(&i_threads[i], nullptr, intermediate, &p[i]);
            assign_thread_to_core(i + NUM_THREAD, i_threads[i]);
        }

        for (unsigned long i_thread : i_threads) {
            pthread_join(i_thread, nullptr);
        }
        printf("Process 2 completed\n");
        exit(0);
    }

    if (fork() == 0) {
        pthread_t r_threads[NUM_THREAD];
        for (int i = 0; i < NUM_THREAD; i++) {
            pthread_create(&r_threads[i], nullptr, receiver, &p[i]);
            assign_thread_to_core(i + NUM_THREAD * 2, r_threads[i]);
        }
        for (unsigned long r_thread : r_threads) {
            pthread_join(r_thread, nullptr);
        }
        printf("Process 3 completed\n");
        exit(0);
    }


    int status;
    while (wait(&status) > 0) std::cout << "Process Exit status: " << status << std::endl;
}


static void *sender(void *par) {
    auto *p = (params *) par;
    MSQueue<size_lt> *q12 = p->q12;
    MSQueue<size_lt> *q21 = p->q21;
    int id = p->id;
    size_lt start[NUM_RUNS];
    size_lt end[NUM_RUNS];
    size_lt delay[NUM_RUNS];
    size_lt time[NUM_RUNS];
    size_lt *res;

    /* for (int i = 0; i < 100; i++) {
         q12->enqueue(&reqs[i], id);
     }*/

    for (int j = 0; j < NUM_RUNS; j++) {
        pthread_barrier_wait(barrier_t);

        delay[j] = elapsed_time_ns(0);
        start[j] = elapsed_time_ns(0);
        for (int i = 0; i < NUM_ITERS; i++) {
            q12->enqueue(&reqs[i], id);
            while ((res = q21->dequeue(id)) == nullptr);
        }
        end[j] = elapsed_time_ns(0);
        if (res != nullptr) printf("Verify S: %llu\n", *res);
        if (res == nullptr) printf("Verify is NULL");

        size_lt timer_overhead = start[j] - delay[j];
        time[j] = end[j] - start[j] - timer_overhead;
        printf("Time taken for #%d: %llu ns | Per iter: %f ns | ID: %d\n", j, time[j],
               ((double) time[j]) / NUM_ITERS, id);
//        printf("Verify S: %llu\n", *res);
    }
    size_lt avg = 0;
    for (unsigned long long j : time) {
        avg += llround(((double) j) / NUM_ITERS);
    }
    printf("Sender completed\n");
    p->buf = llround(((double) avg) / NUM_RUNS);
    return 0;
}

static void *intermediate(void *par) {
    auto *p = (params *) par;
    MSQueue<size_lt> *q12 = p->q12;
    MSQueue<size_lt> *q23 = p->q23;
    MSQueue<size_lt> *q32 = p->q32;
    MSQueue<size_lt> *q21 = p->q21;
    int id = p->id + NUM_THREAD;
    size_lt *res;
    for (int j = 0; j < NUM_RUNS; j++) {
        pthread_barrier_wait(barrier_t);
        for (int i = 0; i < NUM_ITERS; i++) {
            while ((res = q12->dequeue(id)) == nullptr);
            q23->enqueue(res, id);
            while ((res = q32->dequeue(id)) == nullptr);
            q21->enqueue(res, id);
        }
    }
//    printf("Verify: %llu\n", *res);
    printf("Intermediate completed\n");
    return 0;
}

static void *receiver(void *par) {
    auto *p = (params *) par;
    MSQueue<size_lt> *q23 = p->q23;
    MSQueue<size_lt> *q32 = p->q32;
    int id = p->id;
    size_lt *res;
    for (int j = 0; j < NUM_RUNS; j++) {
        pthread_barrier_wait(barrier_t);
        for (int i = 0; i < NUM_ITERS; i++) {
            while ((res = q23->dequeue(id)) == nullptr);
            q32->enqueue(res, id);
        }
    }
    printf("Receiver completed\n");
//    printf("Verify R: %d\n", s23->buf);
    return 0;
}