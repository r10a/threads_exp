cmake_minimum_required(VERSION 2.8)
project(threads_exp)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -g -O3 -D_GNU_SOURCE")

find_package(Boost REQUIRED)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

add_executable(threads_exp main.cpp HazardPointers.hpp MSQueue.hpp)

target_link_libraries(threads_exp -lm -lpthread -lrt)
