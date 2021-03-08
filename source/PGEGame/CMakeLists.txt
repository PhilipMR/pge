cmake_minimum_required(VERSION 3.15)

project(pge_game)

add_library(pge_game
    src/game_camera.cpp
    src/game_entity.cpp
    src/game_light.cpp
    src/game_scene.cpp
    src/game_static_mesh.cpp
    src/game_transform.cpp
)

target_include_directories(pge_game PRIVATE
    ../PGEDiagnostics/include
    ../PGEMath/include
    ../PGEInput/include
    ../PGEGraphics/include
    ../PGEResource/include
  )