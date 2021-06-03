cmake_minimum_required(VERSION 3.15)

project(pge_editor)

add_library(pge_editor
    src/edit_camera.cpp
    src/edit_command.cpp
    src/edit_editor.cpp
    src/edit_editor_win32.cpp
    src/edit_entity.cpp
    src/edit_light.cpp
    src/edit_mesh.cpp
    src/edit_script.cpp
    src/edit_transform.cpp
)

target_include_directories(pge_editor PRIVATE 
    ../External/imgui
    ../PGEAnimation/include
    ../PGECore/include
    ../PGEMath/include
    ../PGEGraphics/include
    ../PGEResource/include
    ../PGEInput/include
    ../PGEGame/include
)