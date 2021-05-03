cmake_minimum_required(VERSION 3.15)

project(pge_core)

add_library(pge_core
    src/core_log.cpp
    src/core_file_utils.cpp
    src/core_display_win32.cpp
)