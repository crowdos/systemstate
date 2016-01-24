cmake_minimum_required(VERSION 2.6)
project(SystemState)
set(CMAKE_CXX_FLAGS "-std=c++0x")

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(plugins)
