cmake_minimum_required(VERSION 3.22)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ../../src
)

add_executable(pendulum
    main.cpp
)

target_include_directories(pendulum BEFORE PRIVATE
    ${CMAKE_SOURCE_DIR}/3rd/bullet3/src
)

target_link_libraries(pendulum PRIVATE
    raylib
    BulletDynamics
    BulletCollision
    LinearMath
)

if(UNIX AND NOT APPLE)
    target_link_libraries(pendulum PRIVATE
       pthread
       dl
    )
endif()
