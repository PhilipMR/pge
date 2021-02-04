cmake_minimum_required(VERSION 3.15)

project(pge_sandbox)

add_executable(pge_sandbox
    src/main.cpp
)

target_include_directories(pge_sandbox PRIVATE 
    ../../External/imgui
    ../../PGEOS/include
    ../../PGEInput/include
    ../../PGEGraphics/include
    ../../PGEMath/include
    ../../PGEResource/include
    ../../PGEDiagnostics/include
    ../../PGEGame/include
)

target_link_libraries(pge_sandbox PRIVATE 
    d3d11.lib
    d3dcompiler.lib
    imgui
    pge_os
    pge_input
    pge_graphics
    pge_resource
    pge_diagnostics
    pge_game
)