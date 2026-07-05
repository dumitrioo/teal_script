cmake_minimum_required(VERSION 3.22)
project(TealScript C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    add_compile_options(/Zc:__cplusplus /bigobj)
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)

#add_definitions(-DTEAL_USE_CUSTOM_SHARED_MUTEX -DRW_MUTEX_PRIORITIES -DRW_MUTEX_UPGRADEABLE)
#add_definitions(-DTEAL_USE_CUSTOM_ANY)
#add_definitions(-DUSE_CUSTOM_MEMORY_ALLOCATION)
#add_definitions(-DSTR_UTIL_ENABLE_CUSTOM_UNICODE_OPERATIONS)

add_subdirectory(examples/interpreter)
add_subdirectory(examples/fractal)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "Build games" FORCE)

add_subdirectory(3rd/bullet3)
add_subdirectory(3rd/raylib)
add_subdirectory(3rd/libzmq)

add_subdirectory(examples/pendulum)
add_subdirectory(examples/ray)
add_subdirectory(examples/zmq)
