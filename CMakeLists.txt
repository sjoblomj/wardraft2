cmake_minimum_required(VERSION 3.25)
project(wardraft2 C)

set(CMAKE_C_STANDARD 11)

add_executable(wardraft2 wd2.c parse_input.c wararchivelib.c wararchivelib.h)
