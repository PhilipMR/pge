cmake_minimum_required(VERSION 3.15)

project(imgui)
add_library(imgui 
    imgui/imgui.cpp    
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp
    imgui/backends/imgui_impl_win32.cpp
    imgui/backends/imgui_impl_dx11.cpp
)

target_include_directories(imgui PRIVATE
        ./
)
