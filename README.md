## DPDK RTE-Ring benchmark

Simulation of three process roundtrip scenario using [RTE_RING](https://dpdk.readthedocs.io/en/v16.04/prog_guide/ring_lib.html).

RTE_Ring is a non-blocking queue part of Intel's Data Plane Development Kit ([DPDK](https://core.dpdk.org/doc/))

DPDK requires all programs to be run in superuser mode. Also ensure [huge-pages](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/6/html/performance_tuning_guide/s-memory-transhuge) are allocated before execution.
### Build instructions
Requires Boost and DPDK libraries installed.

1. Clone repositroy using `git clone https://github.com/r10a/threads_exp.git`
2. Navigate into directory: `cd threads_exp`
3. Checkout RTE_Ring branch: `git checkout RTE_Ring`
4. Build: `cmake . && make`

### Run instrution 
Open 3 different terminals with _sudo_ access and run the following commands on each to start the benchmark
- Terminal 1: `./main -l <core-mask> -n 4`
- Terminal 2: `./main -l <core-mask> -n 4 --proc-type=secondary`
- Terminal 3: `./main -l <core-mask> -n 4 --proc-type=secondary last`

Here core-mask is the cores on which the current process must execute.

For example to run the simulation of 3 threads/process - 
- Terminal 1: `./main -l 0-3 -n 4`
- Terminal 2: `./main -l 3-6 -n 4 --proc-type=secondary`
- Terminal 3: `./main -l 6-9 -n 4 --proc-type=secondary last`

For 4 threads/process - 
- Terminal 1: `./main -l 0-4 -n 4`
- Terminal 2: `./main -l 4-8 -n 4 --proc-type=secondary`
- Terminal 3: `./main -l 8-12 -n 4 --proc-type=secondary last`
