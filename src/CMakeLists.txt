cmake_minimum_required(VERSION 3.0)

aux_source_directory(. DIR_SRCS)

add_library(icecream ${DIR_SRCS})