/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

/*
 * This sample application is a simple multi-process application which
 * demostrates sharing of queues and memory pools between processes, and
 * using those queues/pools for communication between the processes.
 *
 * Application is designed to run with two processes, a primary and a
 * secondary, and each accepts commands on the commandline, the most
 * important of which is "send", which just sends a string to the other
 * process.
 */

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
static const char *_SEC_2_PRI = "SEC_2_PRI";
static const char *_PRI_2_SEC = "PRI_2_SEC";

struct rte_ring *send_ring, *recv_ring;
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
lcore_master(__attribute__((unused)) void *arg) {
    unsigned lcore_id = rte_lcore_id();

    printf("Starting core %u on master\n", lcore_id);
    void *msg = nullptr;
    void *recv = nullptr;
    int ret = 0;

    while ((ret = rte_ring_dequeue(recv_ring, &recv)) < 0);

    printf("Received %s with status: %d on core: %d\n", (char *) recv, ret, lcore_id);
    rte_mempool_put(message_pool, recv);
    int patience = 0;

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
            while (rte_ring_dequeue(recv_ring, &recv) < 0 && patience++ < 1000);
            patience = 0;
//        printf("core %u: Received '%s' %d\n", lcore_id, (char *) recv, patience);
        }
        end[j] = elapsed_time_ns(0);
        size_lt timer_overhead = start[j] - delay[j];
        time[j] = end[j] - start[j] - timer_overhead;
        printf("Time taken for #%d: %llu ns | Per iter: %f ns | ID: %d\n", j, time[j],
               ((double) time[j]) / NUM_ITERS / 2 , lcore_id);
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
lcore_slave(__attribute__((unused)) void *arg) {
    unsigned lcore_id = rte_lcore_id();

    printf("Starting core %u on slave\n", lcore_id);
    int patience = 0;
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
            while (rte_ring_dequeue(recv_ring, &recv) < 0 && patience++ < 10000);
            patience = 0;
            rte_ring_enqueue(send_ring, recv);
        }
    }

//    rte_mempool_put(message_pool, recv);
    printf("Finished %u on slave\n", lcore_id);
    return 0;
}

int main(int argc, char **argv) {
    const unsigned flags = 0;
    const unsigned ring_size = 32768;
    const unsigned pool_size = 16384;
    const unsigned pool_cache = 128;
    const unsigned priv_data_sz = 32;

    int ret;
    unsigned lcore_id;

    ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Cannot init EAL\n");

    if (rte_eal_process_type() == RTE_PROC_PRIMARY) {
        send_ring = rte_ring_create(_PRI_2_SEC, ring_size, rte_socket_id(), flags);
        recv_ring = rte_ring_create(_SEC_2_PRI, ring_size, rte_socket_id(), flags);
        message_pool = rte_mempool_create(_MSG_POOL, pool_size,
                                          STR_TOKEN_SIZE, pool_cache, priv_data_sz,
                                          NULL, NULL, NULL, NULL,
                                          rte_socket_id(), flags);
    } else {
        recv_ring = rte_ring_lookup(_PRI_2_SEC);
        send_ring = rte_ring_lookup(_SEC_2_PRI);
        message_pool = rte_mempool_lookup(_MSG_POOL);
    }
    if (send_ring == NULL)
        rte_exit(EXIT_FAILURE, "Problem getting sending ring\n");
    if (recv_ring == NULL)
        rte_exit(EXIT_FAILURE, "Problem getting receiving ring\n");
    if (message_pool == NULL)
        rte_exit(EXIT_FAILURE, "Problem getting message pool\n");

    RTE_LOG(INFO, APP, "Finished Process Init.\n");

    /* call lcore_recv() on every slave lcore */
    if (rte_eal_process_type() == RTE_PROC_PRIMARY) {
        RTE_LCORE_FOREACH_SLAVE(lcore_id) {
            rte_eal_remote_launch(lcore_master, NULL, lcore_id);
        }
    } else {
        RTE_LCORE_FOREACH_SLAVE(lcore_id) {
            rte_eal_remote_launch(lcore_slave, NULL, lcore_id);
        }
    }

    rte_eal_mp_wait_lcore();
    return 0;
}