cmake_minimum_required(VERSION 3.22)
project(TealScript C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "CMake version: ${CMAKE_VERSION}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "Build games" FORCE)

add_subdirectory(3rd/bullet3)
add_subdirectory(3rd/raylib)
add_subdirectory(3rd/libzmq)

add_subdirectory(examples/interpreter)
add_subdirectory(examples/fractal)
add_subdirectory(examples/pendulum)
add_subdirectory(examples/ray)
add_subdirectory(examples/zmq)
