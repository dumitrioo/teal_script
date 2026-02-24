cmake_minimum_required(VERSION 3.10.2)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ../../src
    ../../include
)

add_library(fractal SHARED
    libmain.cpp
)

target_link_libraries(fractal
    pthread
    dl
)
