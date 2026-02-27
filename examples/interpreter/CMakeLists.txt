cmake_minimum_required(VERSION 3.10.2)

add_definitions(-DUSE_FILE_MAGIC)
add_definitions(-DSCFX_USE_ZMQ)
add_definitions(-DSCFX_USE_RAYLIB)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ../../src
)

add_executable(scaflux
    main.cpp
)

find_package(PkgConfig)
pkg_check_modules(PC_ZeroMQ QUIET zmq)
find_path(ZeroMQ_INCLUDE_DIR
    NAMES zmq.hpp
    PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
)
find_library(ZeroMQ_LIBRARY
    NAMES zmq
    PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
)
if(NOT ZeroMQ_LIBRARY)
    message(STATUS "zeromq not found")
else()
    message(STATUS "zeromq found")
    add_definitions(-DSCFX_USE_ZMQ)
    target_link_libraries(scaflux ${ZeroMQ_LIBRARY})
endif()

find_package(raylib QUIET)
if(raylib_FOUND)
    message(STATUS "raylib found")
    add_definitions(-DSCFX_USE_RAYLIB)
    target_link_libraries(scaflux raylib)
else()
    message(STATUS "raylib not found")
endif()

target_link_libraries(scaflux
    pthread
    dl
    z
    magic
)
