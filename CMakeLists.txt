cmake_minimum_required(VERSION 3.6)

project(IST_test)

set(CMAKE_CXX_STANDARD 20)

add_library(concordance OBJECT concordance.cpp)

add_executable(IST_test main.cpp concordance)
