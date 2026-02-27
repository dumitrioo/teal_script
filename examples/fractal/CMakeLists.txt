cmake_minimum_required(VERSION 3.10.2)

add_library(fractal SHARED
    libmain.cpp
)

target_link_libraries(fractal
    pthread
    dl
)
