cmake_minimum_required(VERSION 2.8)
project(threads_exp C)

set(CMAKE_C_COMPILER "gcc")

set(CMAKE_C_FLAGS "-std=c99 -g -Wall -O3 -D_GNU_SOURCE")

find_package( Threads REQUIRED )

#add_executable(threads_exp main.c ArrayQueue.c ArrayQueue.h)
add_executable(threads_exp align.h bits.h cpumap.h delay.h wfqueue.c wfqueue.h primitives.h pairwise.c main.c)

target_link_libraries(threads_exp -lm -lpthread -lrt)
