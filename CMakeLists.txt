cmake_minimum_required(VERSION 3.7)
project(mtFS)

set(CMAKE_CXX_STANDARD 14)

include_directories(library)

add_subdirectory(mtFS)
add_subdirectory(mtFS_Tests)
add_subdirectory(Plugin)