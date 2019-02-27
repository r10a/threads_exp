## CRTQueue queue benchmark

Simulation of three process roundtrip scenario using [MSQueue](http://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf).

Queue implementation modified for shared memory using [Boost](https://www.boost.org/doc/libs/1_63_0/doc/html/interprocess.html) library.

CRTurn queue is a memory-unbounded multi-producer-multi-consumer wait-free queue for C++, that does its own memory reclamation, and does it in a wait-free way.

Default simulation runs with 8 threads per process, i.e., 24 threads total, each pinned to different core.
Hence, the processor should have at least **24** cores to run this simulation.

To lower the number of threads per process, modify  `#define NUM_THREAD 8` in `main.cpp` on line 17.

### Build instructions
Requires Boost libraries installed.

1. Clone repositroy using `git clone https://github.com/r10a/threads_exp.git`
2. Navigate into directory: `cd threads_exp`
3. Checkout CRTQueue branch: `git checkout CRTQueue`
4. Build: `cmake . && make`
5. Run `./threads_exp`
