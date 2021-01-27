cmake_minimum_required(VERSION 3.15)

project(pge_graphics)

add_library(pge_graphics 
    src/gfx_graphics_adapter_d3d11.cpp
    src/gfx_graphics_device_d3d11.cpp
    src/gfx_shader_d3d11.cpp
    src/gfx_buffer_d3d11.cpp
    src/gfx_vertex_layout_d3d11.cpp
    src/gfx_texture_d3d11.cpp
    src/gfx_sampler_d3d11.cpp
)

target_include_directories(pge_graphics PRIVATE 
    ../PGEOS/include
    ../PGEDiagnostics/include
)