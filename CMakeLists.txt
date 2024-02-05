cmake_minimum_required(VERSION 3.20)
project(wordle_solver LANGUAGES "CXX")

add_executable(wordle wordle.cpp)
set_property(TARGET wordle PROPERTY CXX_STANDARD 23)