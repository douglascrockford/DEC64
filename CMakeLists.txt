cmake_minimum_required(VERSION 3.10)

project(dec64)

set(CMAKE_CFLAGS $CMAKE_CFLAGS "-Wall")
add_library(dec64 STATIC dec64.h dec64.c dec64_string.h dec64_string.c dec64_math.h dec64_math.c)
add_executable(dec64_test dec64_test.h dec64_main_test.c dec64_test.c dec64_math_test.c dec64_string_test.c)
target_link_libraries(dec64_test dec64)
