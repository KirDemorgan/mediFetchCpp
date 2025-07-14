cmake_minimum_required(VERSION 3.30)
project(mediFetchCpp)

set(CMAKE_CXX_STANDARD 20)

if(MSVC)
    add_compile_options(/FS)
endif()

add_executable(mediFetchCpp src/main.cpp)