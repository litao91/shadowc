cmake_minimum_required (VERSION 3.1)
project (shadowc)
include_directories ("${PROJECT_SOURCE_DIR}/include")

add_executable(server "${PROJECT_SOURCE_DIR}/src/server.cpp")
target_compile_options(server PUBLIC -std=c++11 -Wall)

