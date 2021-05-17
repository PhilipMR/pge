cmake_minimum_required(VERSION 3.15)

project(pge_animation)

add_library(pge_animation
    src/anim_skeleton.cpp
)

target_include_directories(pge_animation PRIVATE
    ../PGECore/include
    ../PGEMath/include
)