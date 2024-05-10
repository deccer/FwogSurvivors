cmake_minimum_required(VERSION 3.22)

project(FwogSurvivors)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "")

add_subdirectory(lib)
add_subdirectory(src)