cmake_minimum_required(VERSION 2.8)
project(threads_exp C)

set(CMAKE_C_FLAGS "-std=c99")

find_package( Threads REQUIRED )

#add_executable(threads_exp main.c ArrayQueue.c ArrayQueue.h)

add_executable(threads_exp main.c CircularQueue.c CircularQueue.h utils.c utils.h)
#add_executable(threads_exp utils.c utils.h benchmarks/posix_mutex.c benchmarks/posix_mutex.h)
#add_executable(threads_exp utils.c utils.h benchmarks/pipe.c benchmarks/pipe.h)
#add_executable(threads_exp utils.c utils.h benchmarks/futex.c benchmarks/futex.h)
#add_executable(threads_exp utils.c utils.h benchmarks/futex_local.c benchmarks/futex.h)
#add_executable(threads_exp utils.c utils.h benchmarks/signal_kill.c benchmarks/signal_kill.h)

target_link_libraries(threads_exp -lpthread -lrt)
