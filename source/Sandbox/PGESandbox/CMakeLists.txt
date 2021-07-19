cmake_minimum_required(VERSION 3.15)

project(pge_sandbox)

add_executable(pge_sandbox WIN32
    src/main.cpp
)

add_definitions(-DNOMINMAX)

target_compile_options(pge_sandbox PRIVATE /W3)

target_include_directories(pge_sandbox PRIVATE 
    ../../External/imgui
    ../../External/lua
    ../../PGEAnimation/include
    ../../PGECore/include
    ../../PGEInput/include
    ../../PGEGraphics/include
    ../../PGEMath/include
    ../../PGEResource/include
    ../../PGEGame/include
    ../../PGEEditor/include
)

target_link_libraries(pge_sandbox PRIVATE 
    d3d11.lib
    d3dcompiler.lib
    imgui
    lua
    pge_animation
    pge_core
    pge_input
    pge_graphics
    pge_resource
    pge_game
    pge_editor
)