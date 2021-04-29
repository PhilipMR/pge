cmake_minimum_required(VERSION 3.15)

project(pge_core)

add_library(pge_core
    src/core_log.cpp
    src/core_fileutils.cpp
    src/core_display_win32.cpp
)