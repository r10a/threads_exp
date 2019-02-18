#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <algorithm>
#include <sys/queue.h>

#include <rte_common.h>
#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <rte_atomic.h>
#include <rte_branch_prediction.h>
#include <rte_ring.h>
#include <rte_log.h>
#include <rte_mempool.h>
#include <cmdline_rdline.h>
#include <cmdline_parse.h>
#include <cmdline_parse_string.h>
#include <cmdline_socket.h>
#include <cmdline.h>

#define RTE_LOGTYPE_APP RTE_LOGTYPE_USER1

static const char *_MSG_POOL = "MSG_POOL";
static const char *_PRI_2_INTE = "PRI_2_INTE";
static const char *_INTE_2_LAST = "INTE_2_LAST";
static const char *_LAST_2_INTE = "LAST_2_INTE";
static const char *_INTE_2_PRI = "INTE_2_PRI";

struct rte_ring *send_ring, *recv_ring;
struct rte_ring *send_ring1, *recv_ring1;
struct rte_mempool *message_pool;

#define NUM_ITERS 1000000
#define NUM_RUNS 5
#define size_lt unsigned long long

static inline size_lt elapsed_time_ns(size_lt ns) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000000000L + t.tv_nsec - ns;
}

static int
lcore_primary(__attribute__((unused)) void *arg) {
    unsigned lcore_id = rte_lcore_id();

    printf("Starting core %u on master\n", lcore_id);
    void *msg = nullptr;
    void *recv = nullptr;
    int ret = 0;

    while ((ret = rte_ring_dequeue(recv_ring, &recv)) < 0);

    printf("Received %s with status: %d on core: %d\n", (char *) recv, ret, lcore_id);
    rte_mempool_put(message_pool, recv);

    if (rte_mempool_get(message_pool, &msg) < 0)
        rte_panic("Failed to get message buffer\n");

    size_lt start[NUM_RUNS];
    size_lt end[NUM_RUNS];
    size_lt delay[NUM_RUNS];
    size_lt time[NUM_RUNS];

    for (int j = 0; j < NUM_RUNS; j++) {
        delay[j] = elapsed_time_ns(0);
        start[j] = elapsed_time_ns(0);
        for (int i = 0; i < NUM_ITERS; i++) {
            snprintf((char *) msg, STR_TOKEN_SIZE, "%d", i);
            rte_ring_enqueue(send_ring, msg);
            while (rte_ring_dequeue(recv_ring, &recv) < 0 /*&& patience++ < PATIENCE*/);
//            printf("core %u: Received '%s' %d\n", lcore_id, (char *) recv, patience);
        }
        end[j] = elapsed_time_ns(0);
        size_lt timer_overhead = start[j] - delay[j];
        time[j] = end[j] - start[j] - timer_overhead;
        printf("Time taken for #%d: %llu ns | Per iter: %f ns | ID: %d\n", j, time[j],
               ((double) time[j]) / NUM_ITERS / 2, lcore_id);
    }
    rte_mempool_put(message_pool, msg);

    printf("Finished %u on master\n", lcore_id);
    size_lt avg = 0;
    for (unsigned long long j : time) {
        avg += llround(((double) j) / NUM_ITERS);
    }
    printf("Thread %d average: %llu\n", lcore_id, llround(((double) avg) / NUM_RUNS / 2));
    return 0;
}

static int
lcore_intermediate(__attribute__((unused)) void *arg) {
    unsigned lcore_id = rte_lcore_id();

    printf("Starting core %u on intermediate\n", lcore_id);
    void *msg = nullptr;
    void *start = nullptr;
    int ret = 0;

    if (rte_mempool_get(message_pool, &msg) < 0)
        rte_panic("Failed to get message buffer\n");

    while ((ret = rte_ring_dequeue(recv_ring1, &start)) < 0);
    printf("Received %s with status: %d on core: %d\n", (char *) start, ret, lcore_id);
    rte_mempool_put(message_pool, start);

    snprintf((char *) msg, STR_TOKEN_SIZE, "%s", "starting");
    ret = rte_ring_enqueue(send_ring, msg);
    printf("Sent %s with status: %d on core: %d\n", (char *) msg, ret, lcore_id);

    for (int j = 0; j < NUM_RUNS; j++) {
        for (int i = 0; i < NUM_ITERS; i++) {
            void *recv = nullptr;
            while (rte_ring_dequeue(recv_ring, &recv) < 0 /*&& patience++ < PATIENCE*/);
            rte_ring_enqueue(send_ring1, recv);
            void* recv1 = nullptr;
            while (rte_ring_dequeue(recv_ring1, &recv1) < 0 /*&& patience++ < PATIENCE*/);
            rte_ring_enqueue(send_ring, recv1);
        }
    }

//    rte_mempool_put(message_pool, recv);
    printf("Finished %u on intermediate\n", lcore_id);
    return 0;
}

static int
lcore_last(__attribute__((unused)) void *arg) {
    unsigned lcore_id = rte_lcore_id();

    printf("Starting core %u on last\n", lcore_id);
    void *msg = nullptr;

    int ret = 0;
    if (rte_mempool_get(message_pool, &msg) < 0)
        rte_panic("Failed to get message buffer\n");

    snprintf((char *) msg, STR_TOKEN_SIZE, "%s", "starting");
    ret = rte_ring_enqueue(send_ring, msg);
    printf("Sent %s with status: %d on core: %d\n", (char *) msg, ret, lcore_id);

    for (int j = 0; j < NUM_RUNS; j++) {
        for (int i = 0; i < NUM_ITERS; i++) {
            void *recv = nullptr;
            while (rte_ring_dequeue(recv_ring, &recv) < 0 /*&& patience++ < PATIENCE*/);
//            printf("core %u: Received '%s' %d\n", lcore_id, (char *) recv, patience);
            rte_ring_enqueue(send_ring, recv);
        }
    }

//    rte_mempool_put(message_pool, recv);
    printf("Finished %u on last\n", lcore_id);
    return 0;
}

int main(int argc, char **argv) {
    const unsigned flags = 0;
    const unsigned ring_size = 32768;
    const unsigned pool_size = 16384;
    const unsigned pool_cache = 128;
    const unsigned priv_data_sz = 0;

    int ret;
    unsigned lcore_id;

    bool intermediate = false;

    /**
     * Roundtrip: Primary > Intermediate > Last > Intermediate > Primary
     */

    if (argc == 5) { // Primary Process
        ret = rte_eal_init(argc, argv);
    } else if (argc == 7) { // Last Process
        ret = rte_eal_init(6, argv);
    } else { // Intermediate process
        intermediate = true;
        ret = rte_eal_init(6, argv);
    }

    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Cannot init EAL\n");

    if (rte_eal_process_type() == RTE_PROC_PRIMARY) {
        send_ring = rte_ring_create(_PRI_2_INTE, ring_size, rte_socket_id(), flags);
        send_ring1 = rte_ring_create(_INTE_2_LAST, ring_size, rte_socket_id(), flags);
        recv_ring1 = rte_ring_create(_LAST_2_INTE, ring_size, rte_socket_id(), flags);
        recv_ring = rte_ring_create(_INTE_2_PRI, ring_size, rte_socket_id(), flags);

        message_pool = rte_mempool_create(_MSG_POOL, pool_size,
                                          STR_TOKEN_SIZE, pool_cache, priv_data_sz,
                                          nullptr, nullptr, nullptr, nullptr,
                                          rte_socket_id(), flags);
    } else {
        if (intermediate) {
            recv_ring = rte_ring_lookup(_PRI_2_INTE);
            send_ring1 = rte_ring_lookup(_INTE_2_LAST);
            recv_ring1 = rte_ring_lookup(_LAST_2_INTE);
            send_ring = rte_ring_lookup(_INTE_2_PRI);
        } else {
            recv_ring = rte_ring_lookup(_INTE_2_LAST);
            send_ring = rte_ring_lookup(_LAST_2_INTE);
            send_ring1 = send_ring;
            recv_ring1 = recv_ring;
        }
        message_pool = rte_mempool_lookup(_MSG_POOL);
    }
    if (send_ring == nullptr || send_ring1 == nullptr)
        rte_exit(EXIT_FAILURE, "Problem getting sending ring\n");
    if (recv_ring == nullptr || recv_ring1 == nullptr)
        rte_exit(EXIT_FAILURE, "Problem getting receiving ring\n");
    if (message_pool == nullptr)
        rte_exit(EXIT_FAILURE, "Problem getting message pool\n");

    RTE_LOG(INFO, APP, "Finished Process Init.\n");

    if (rte_eal_process_type() == RTE_PROC_PRIMARY) {
        RTE_LCORE_FOREACH_SLAVE(lcore_id) {
            rte_eal_remote_launch(lcore_primary, nullptr, lcore_id);
        }
    } else {
        if (intermediate) {
            RTE_LCORE_FOREACH_SLAVE(lcore_id) {
                rte_eal_remote_launch(lcore_intermediate, nullptr, lcore_id);
            }
        } else {
            RTE_LCORE_FOREACH_SLAVE(lcore_id) {
                rte_eal_remote_launch(lcore_last, nullptr, lcore_id);
            }
        }
    }

    rte_eal_mp_wait_lcore();
    return 0;
}