cmake_minimum_required(VERSION 3.0)

project(icecream)

set(CMAKE_BUILD_TYPE Debug)

add_library(icecream socket.cpp)