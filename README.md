## LCRQ benchmark

Simulation of three process roundtrip scenario using [LCRQ](http://www.cs.tau.ac.il/~mad/publications/ppopp2013-x86queues.pdf).
![round-trip scenario](/pictures/scenario-roundtrip.png)

Queue implementation modified for shared memory using [Boost](https://www.boost.org/doc/libs/1_63_0/doc/html/interprocess.html) library.

LCRQ is a non-blocking Multi-producer multi-consumer queue.

Default simulation runs with 8 threads per process, i.e., 24 threads total, each pinned to different core.
Hence, the processor should have at least 24 cores to run this simulation.

To lower the number of threads per process, modify  `#define NUM_THREAD 8` in `main.cpp` on line 18.

### Build instructions
Requires Boost libraries to be installed.

1. Clone repositroy using `git clone https://github.com/r10a/threads_exp.git`
2. Navigate into directory: `cd threads_exp`
3. Checkout LCRQueue branch: `git checkout LCRQueue`
4. Build: `cmake . && make`
5. Run `./threads_exp`
