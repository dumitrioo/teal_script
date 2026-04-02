cmake_minimum_required(VERSION 3.10.2)

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
)

find_library(magic_LIBRARY magic)
if(NOT magic_LIBRARY)
    message(STATUS "magic not found")
else()
    message(STATUS "magic found")
    add_definitions(-DUSE_FILE_MAGIC)
    target_link_libraries(scaflux magic)
endif()
