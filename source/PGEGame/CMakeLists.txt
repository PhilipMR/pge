cmake_minimum_required(VERSION 3.15)

project(pge_game)

add_library(pge_game
    src/game_animation.cpp
    src/game_behaviour.cpp
    src/game_camera.cpp
    src/game_entity.cpp
    src/game_light.cpp
    src/game_renderer.cpp
    src/game_world.cpp
    src/game_script.cpp
    src/game_mesh.cpp
    src/game_transform.cpp
)

target_include_directories(pge_game PRIVATE
    ../External/lua
    ../PGEAnimation/include
    ../PGECore/include
    ../PGEMath/include
    ../PGEInput/include
    ../PGEGraphics/include
    ../PGEResource/include
  )