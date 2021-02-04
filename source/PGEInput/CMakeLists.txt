cmake_minimum_required(VERSION 3.15)

project(pge_input)

add_library(pge_input
    src/input_mouse_win32.cpp
    src/input_keyboard_win32.cpp
)

target_include_directories(pge_input PRIVATE
    ../PGEMath/include
)