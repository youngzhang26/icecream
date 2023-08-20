cmake_minimum_required(VERSION 3.0)

add_executable(testGetAddrInfo testGetAddrInfo.cpp)

include_directories(../)

add_executable(client client.cpp )
target_link_libraries(client icecream)

add_executable(server server.cpp)
target_link_libraries(server icecream)

