cmake_minimum_required(VERSION 3.15)

project(pge_editor)

add_library(pge_editor
    src/edit_editor_win32.cpp
)

target_include_directories(pge_editor PRIVATE 
    ../External/imgui
    ../PGEDiagnostics/include
    ../PGEOS/include
    ../PGEGraphics/include
)