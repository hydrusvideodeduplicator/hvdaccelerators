cmake_minimum_required(VERSION 3.15...3.26)
project(${SKBUILD_PROJECT_NAME} LANGUAGES CXX)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

# This compiles pdq which produces the library file that will be linked below.
add_subdirectory(pdq)

pybind11_add_module(stuff MODULE src/main.cpp)

target_link_libraries(stuff PUBLIC pdqlib)

install(TARGETS stuff DESTINATION ${SKBUILD_PROJECT_NAME})
