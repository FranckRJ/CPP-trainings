cmake_minimum_required(VERSION 3.16)
project(expektid)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

add_executable(expektid main.cpp expektid.hpp)
