cmake_minimum_required(VERSION 3.1)
project(main)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++11")

set(SOURCE_FILES main.cpp)

find_package(FLTK)

include_directories(.)
link_directories(
    /usr/lib/x86_64-linux-gnu/
    /usr/include/libpng12
)

add_executable(main ${SOURCE_FILES})

target_link_libraries(main pthread ${FLTK_LIBRARIES})

