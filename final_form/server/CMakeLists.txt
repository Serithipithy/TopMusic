cmake_minimum_required(VERSION 3.17)
project(server)

set(CMAKE_CXX_STANDARD 14)

add_executable(server mainServer.cpp serverHandler.cpp serverHandler.h ceva.cpp kk.cpp)