cmake_minimum_required(VERSION 3.22)

add_library(ray SHARED
    libmain.cpp
)

target_link_libraries(ray PRIVATE
    raylib
)

if(UNIX AND NOT APPLE)
    target_link_libraries(ray PRIVATE pthread dl)
endif()
