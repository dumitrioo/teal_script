cmake_minimum_required(VERSION 3.10.2)

add_definitions(-DUSE_FILE_MAGIC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ../../src
)

add_executable(scaflux
    main.cpp
)

target_link_libraries(scaflux
    pthread
    dl
    z
    magic
)

find_library(ZeroMQ_LIBRARY zmq)
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
