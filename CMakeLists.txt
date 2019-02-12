cmake_minimum_required(VERSION 2.8)
project(threads_exp)

set(CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package (dpdk REQUIRED)
include_directories (${DPDK_INCLUDE_DIR})
include_directories (${CMAKE_SOURCE_DIR}/include)

set (RTE_TARGET x86_64-native-linuxapp-gcc)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -O3 -Wall -Wextra -Wno-unused-parameter -ggdb -g -mssse3")

find_package(Boost REQUIRED)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

aux_source_directory (${CMAKE_CURRENT_SOURCE_DIR} SRCS)

foreach (SRC_PATH ${SRCS})
    get_filename_component (SRC ${SRC_PATH} NAME_WE)
    message (${SRC})
    add_executable (${SRC} ${SRCS})
    target_link_libraries (${SRC} ${DPDK_LIBRARIES})
    set_target_properties (${SRC} PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS})
endforeach (SRC_PATH)