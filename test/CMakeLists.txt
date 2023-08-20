cmake_minimum_required(VERSION 3.0)

project(test)

set(CMAKE_BUILD_TYPE Debug)

add_executable(testGetAddrInfo testGetAddrInfo.cpp)

add_executable(client client.cpp)