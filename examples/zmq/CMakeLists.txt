cmake_minimum_required(VERSION 3.22)

add_library(zeromq SHARED
    libmain.cpp
)

target_link_libraries(zeromq PRIVATE
    zmq
)
