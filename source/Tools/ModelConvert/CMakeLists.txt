cmake_minimum_required(VERSION 3.15)

project(model_convert)

add_executable(model_convert
    src/main.cpp
)

target_include_directories(model_convert PRIVATE
    ../../External/assimp
    ../../External/stb
    ../../PGEAnimation/include
    ../../PGECore/include
    ../../PGEResource/include
    ../../PGEGraphics/include
    ../../PGEMath/include
)

target_link_libraries(model_convert PRIVATE
    pge_animation
    pge_core
    pge_resource
    pge_graphics
    d3d11.lib
    d3dcompiler.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../External/assimp/lib/x64/assimp-vc142-mtd.lib
)