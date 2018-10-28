cmake_minimum_required(VERSION 3.12)
project(threads_exp C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -lrt")

find_package( Threads REQUIRED )

add_executable(threads_exp main.c)

target_link_libraries(threads_exp Threads::Threads)

find_library(LIBRT rt)
if(LIBRT)
    target_link_libraries(threads_exp ${LIBRT})
endif()