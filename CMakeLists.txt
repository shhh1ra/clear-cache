cmake_minimum_required(VERSION 4.0)
project(clear-cache)

set(CMAKE_CXX_STANDARD 23)

add_executable(clear-cache main.cpp)
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++ -s")
