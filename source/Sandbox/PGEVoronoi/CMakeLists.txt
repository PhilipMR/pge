cmake_minimum_required(VERSION 3.15)

project(pge_voronoi)

add_executable(pge_voronoi
    src/main.cpp
)

add_definitions(-DNOMINMAX)

target_include_directories(pge_voronoi PRIVATE
    ../../External/imgui
    ../../PGEOS/include
    ../../PGEInput/include
    ../../PGEGraphics/include
    ../../PGEMath/include
    ../../PGEResource/include
    ../../PGEDiagnostics/include
    ../../PGEGame/include
    ../../PGEEditor/include
)

target_link_libraries(pge_voronoi PRIVATE
    d3d11.lib
    d3dcompiler.lib
    imgui
    pge_os
    pge_input
    pge_graphics
    pge_resource
    pge_diagnostics
)