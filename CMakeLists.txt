cmake_minimum_required(VERSION 3.10)
project(ashtonite)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(SOURCES
    src/main.cpp
    src/server.cpp
    src/http.cpp
    src/file_handler.cpp
    src/thread_pool.cpp
    src/config.cpp
)

set(HEADERS
    include/server.h
    include/http.h
    include/file_handler.h
    include/thread_pool.h
    include/config.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE include)

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
