project (test_pge_math)

add_executable(test_pge_math
    test_math_vec2.cpp
    test_math_vec3.cpp
    test_math_vec4.cpp
    test_math_quat.cpp
    test_math_mat4x4.cpp
)
target_link_libraries(test_pge_math
    gtest gtest_main
    pge_core
)
target_include_directories(test_pge_math PRIVATE
    ../../PGECore/include
    ../../PGEMath/include
)
