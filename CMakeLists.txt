cmake_minimum_required(VERSION 3.5)
project(threads_exp C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -lrt")

find_package( Threads REQUIRED )

#add_executable(threads_exp main.c ArrayQueue.c ArrayQueue.h)

#add_executable(threads_exp main.c CircularQueue.c CircularQueue.h utils.c utils.h)
#add_executable(threads_exp utils.c utils.h benchmarks/posix_mutex.c benchmarks/posix_mutex.h)
#add_executable(threads_exp utils.c utils.h benchmarks/pipe.c benchmarks/pipe.h)
#add_executable(threads_exp utils.c utils.h benchmarks/futex.c benchmarks/futex.h)
add_executable(threads_exp utils.c utils.h benchmarks/futex_local.c benchmarks/futex.h)

target_link_libraries(threads_exp Threads::Threads)

find_library(LIBRT rt)
if(LIBRT)
    target_link_libraries(threads_exp ${LIBRT})
endif()