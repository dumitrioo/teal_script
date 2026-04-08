cmake_minimum_required(VERSION 3.10.2)

message(STATUS "CMake version: ${CMAKE_VERSION}")

#project(
#    tealscript
#    VERSION 1.0.1
#    LANGUAGES CXX C
#)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -ftree-vectorize -mavx2 -ftree-vectorizer-verbose=5 -fstrict-aliasing -Wall -Wextra -rdynamic -fPIC -Wno-unused-function -Wno-unused-parameter -Wl,-rpath,.")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -falign-loops -fstrict-aliasing -Wall -Wextra -fPIC -Wl,-rpath,.")

###############################################################################
#add_definitions(-DTEAL_USE_ASYNC_CONSOLE)
###############################################################################

###############################################################################
add_definitions(-DTEAL_USE_EXTERNAL_VALUES)
###############################################################################

###############################################################################
#add_definitions(-DTEAL_USE_CUSTOM_MUTEX)
add_definitions(-DMUTEX_COPYABLE_WITHOUT_ACTUAL_COPYING)
add_definitions(-DMUTEX_ATOMIC_SM_SLEEP_NANOS=100000)
###############################################################################

###############################################################################
#add_definitions(-DTEAL_USE_CUSTOM_SHARED_MUTEX)
add_definitions(-DRW_MUTEX_PRIORITIES)
#add_definitions(-DRW_MUTEX_UPGRADEABLE)
add_definitions(-DRW_MUTEX_COPYABLE_WITHOUT_ACTUAL_COPYING)
#add_definitions(-DRW_MUTEX_ATOMIC_SM_SLEEP_NANOS=10000)
###############################################################################

###############################################################################
add_definitions(-DTEAL_USE_EMHASH8_MAP)
###############################################################################

###############################################################################
#add_definitions(-DSTR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS)
###############################################################################

add_subdirectory(examples/interpreter)
add_subdirectory(examples/fractal)
find_package(raylib QUIET)
if(raylib_FOUND)
    add_subdirectory(examples/ray)
else()
    message(STATUS "raylib not found")
endif()
find_library(ZeroMQ_LIBRARY zmq)
if(NOT ZeroMQ_LIBRARY)
    message(STATUS "zeromq not found")
else()
    add_subdirectory(examples/zmq)
endif()
