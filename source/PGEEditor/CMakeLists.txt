cmake_minimum_required(VERSION 3.15)

project(pge_editor)

add_executable(pge_editor
    src/main.cpp
)

target_include_directories(pge_editor PRIVATE 
    ../External/imgui
    ../PGEOS/include
    ../PGEGraphics/include
)

target_link_libraries(pge_editor PRIVATE 
    d3d11.lib
    imgui
    pge_os
    pge_graphics
)