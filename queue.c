//
// Created by rohit on 1/19/2019.
//

#include "queue.h"

#ifndef ORIGINAL
#include "wfqueue.h"

static queue_t * q;
static handle_t ** hds;

void init_queue(int nprocs) {

    q = align_malloc(PAGE_SIZE, sizeof(queue_t));
    queue_init(q, nprocs);

    hds = align_malloc(PAGE_SIZE, sizeof(handle_t * [nprocs]));
}

void thread_init(int id) {
    hds[id] = align_malloc(PAGE_SIZE, sizeof(handle_t));
    queue_register(q, hds[id], id);
}

void thread_exit(int id) {
    queue_free(q, hds[id]);
}

static inline void wfenqueue(int id, void* val) {
    handle_t * th = hds[id];
    enqueue(q, th, val);
}

static inline void* wfdequeue(int id) {
    handle_t * th = hds[id];
    return dequeue(q, th);
}

#else

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "delay.h"
#include "wfqueue.h"

#ifndef LOGN_OPS
#define LOGN_OPS 5
#endif

static long nops;
static queue_t * q;
static handle_t ** hds;

void init_queue(int nprocs, int logn) {

  /** Use 10^7 as default input size. */
  if (logn == 0) logn = LOGN_OPS;

  /** Compute the number of ops to perform. */
  nops = 1;
  int i;
  for (i = 0; i < logn; ++i) {
    nops *= 10;
  }

  printf("  Number of operations: %ld\n", nops);

  q = align_malloc(PAGE_SIZE, sizeof(queue_t));
  queue_init(q, nprocs);

  hds = align_malloc(PAGE_SIZE, sizeof(handle_t * [nprocs]));
}

void thread_init(int id, int nprocs) {
  hds[id] = align_malloc(PAGE_SIZE, sizeof(handle_t));
  queue_register(q, hds[id], id);
}

void * benchmark(int id, int nprocs) {
  void * val = (void *) (intptr_t) (id + 1);
  handle_t * th = hds[id];

  delay_t state;
  delay_init(&state, id);

  int i;
  for (i = 0; i < nops / nprocs; ++i) {
    enqueue(q, th, val);
    delay_exec(&state);

    val = dequeue(q, th);
    delay_exec(&state);
  }

  return val;
}

void thread_exit(int id, int nprocs) {
  queue_free(q, hds[id]);
}

#ifdef VERIFY
static int compare(const void * a, const void * b) {
  return *(long *) a - *(long *) b;
}
#endif

int verify(int nprocs, void ** results) {
#ifndef VERIFY
  return 0;
#else
  qsort(results, nprocs, sizeof(void *), compare);

  int i;
  int ret = 0;

  for (i = 0; i < nprocs; ++i) {
    int res = (int) (intptr_t) results[i];
    if (res != i + 1) {
      fprintf(stderr, "expected %d but received %d\n", i + 1, res);
      ret = 1;
    }
  }

  if (ret != 1) fprintf(stdout, "PASSED\n");
  return ret;
#endif
}
#endif