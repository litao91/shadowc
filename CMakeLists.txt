cmake_minimum_required (VERSION 2.6)
project (shadowc)
include_directories ("${PROJECT_SOURCE_DIR}/include")
add_executable(server "${PROJECT_SOURCE_DIR}/src/server.cpp")
