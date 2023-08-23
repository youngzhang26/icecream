cmake_minimum_required(VERSION 3.0)

add_executable(test_getaddrinfo test_getaddrinfo.cpp)

include_directories(../)

add_executable(client client.cpp )
target_link_libraries(client icecream)

add_executable(server server.cpp)
target_link_libraries(server icecream)

add_executable(client_hp client_hp.cpp)
target_link_libraries(client_hp icecream)