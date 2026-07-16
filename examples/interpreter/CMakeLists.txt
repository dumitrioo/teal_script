cmake_minimum_required(VERSION 3.22)

add_definitions(-DTEAL_USE_EIGEN)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ../../src
)

add_executable(tealscript
    main.cpp
)

if(UNIX AND NOT APPLE)
    target_link_libraries(tealscript
        pthread
        dl
    )
endif()
