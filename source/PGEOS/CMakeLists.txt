cmake_minimum_required(VERSION 3.15)

project(pge_os)

set(SRC_WIN32
    src/os_display_win32.cpp
)

if (WIN32)
    add_library(pge_os ${SRC_WIN32})
else()
    message(FATAL_ERROR "No supported OS is configured in the build, CMake will exit.")
endif()

target_include_directories(pge_os PRIVATE
    ../PGEDiagnostics/include
)

target_link_libraries(pge_os PRIVATE
    pge_diagnostics
)