cmake_minimum_required(VERSION 3.15...3.26)
project(${SKBUILD_PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ version selection")  # or 11, 14, 17, 20
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

pybind11_add_module(stuff MODULE src/main.cpp)

target_link_libraries(stuff PUBLIC pdqlib)
target_link_libraries(stuff PUBLIC vpdqlib)

install(TARGETS stuff DESTINATION ${SKBUILD_PROJECT_NAME})
