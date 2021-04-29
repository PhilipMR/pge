cmake_minimum_required(VERSION 3.15)

project(pge_resource)

add_library(pge_resource
    src/res_effect.cpp
    src/res_material.cpp
    src/res_texture2d.cpp
    src/res_mesh.cpp
    src/res_resource_manager.cpp
)

target_include_directories(pge_resource PRIVATE
    ../External/stb
    ../PGECore/include
    ../PGEMath/include
    ../PGEGraphics/include
)