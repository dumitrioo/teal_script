cmake_minimum_required(VERSION 3.10.2)

message(STATUS "CMake version: ${CMAKE_VERSION}")

project(
    scaflux
    VERSION 1.0.1
    LANGUAGES CXX C
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=znver4 -fstrict-aliasing -Wall -Wextra -rdynamic -fPIC -Wno-unused-function -Wl,-rpath,.")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -std=c99 -falign-loops -march=znver4 -fstrict-aliasing -Wall -Wextra -fPIC -Wl,-rpath,.")

add_subdirectory(examples/fractal)
add_subdirectory(examples/interpreter)
