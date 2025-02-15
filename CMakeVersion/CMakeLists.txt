cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (lab1)

option(IS_FLOAT "True if you want to compile with float version of array" False)

include_directories(build)

if (IS_FLOAT)
    set(IS_FLOATT true)
else()
    set(IS_FLOATT false)
endif()
