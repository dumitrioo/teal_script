cmake_minimum_required(VERSION 3.10.2)

#add_definitions(-DSINGLE_THREADED_SCFX)
#add_definitions(-DDEBUG_SCFX_RUN_CYCLE)
add_definitions(-DUSE_FILE_MAGIC)
add_definitions(-DSCFX_USE_ZMQ)
add_definitions(-DSCFX_USE_RAYLIB)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ../src
    ../include
    ../ext
)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}
    /tmp/pq/lib
    /tmp/pqxx/lib
    /tmp/openssl/lib
    /tmp/openssl/lib64
)

add_executable(scaflux
    main.cpp
)

target_link_libraries(scaflux
    pthread
    dl
    z
    magic
    raylib
    zmq
)
