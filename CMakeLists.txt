cmake_minimum_required(VERSION 3.15...3.26)
project(${SKBUILD_PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ version selection")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

# Find Threads for C++ multithreading
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# This compiles pdq which produces the library file that will be linked below.
add_subdirectory(pdq)

# This compiles pdq which produces the library file that will be linked below.
add_subdirectory(vpdq)

pybind11_add_module(vpdq MODULE src/main.cpp)

target_link_libraries(vpdq PUBLIC pdqlib)
target_link_libraries(vpdq PUBLIC vpdqlib)

install(TARGETS vpdq DESTINATION ${SKBUILD_PROJECT_NAME})
