cmake_minimum_required(VERSION 3.10)
project(Ashtonite)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

set(SOURCES
    src/main.cpp
    src/server.cpp
)

add_executable(ashtonite ${SOURCES})
