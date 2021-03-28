#include <gtest/gtest.h>
#include <math_mat4x4.h>

using namespace pge;

TEST(math_Mat4x4, Constants)
{
    math_Mat4x4 mat = math_Mat4x4::Identity();
    for (size_t row = 0; row < 4; ++row) {
        for (size_t col = 0; col < 4; ++col) {
            EXPECT_EQ(mat[row][col], row == col ? 1 : 0);
        }
    }
}

TEST(math_Mat4x4, Integrity)
{
    // clang-format off
    math_Mat4x4 mat(11, 12, 13, 14,
                    21, 22, 23, 24,
                    31, 32, 33, 34,
                    41, 42, 43, 44);
    // clang-format on
    for (size_t row = 0; row < 4; ++row) {
        EXPECT_EQ(mat[row], mat.rows[row]);
        for (size_t col = 0; col < 4; ++col) {
            EXPECT_EQ(mat[row][col], (row + 1) * 10 + (col + 1));
            EXPECT_EQ(mat[row][col], mat.values[row * 4 + col]);
        }
    }
}

TEST(math_Mat4x4, Equality)
{
    // clang-format off
    math_Mat4x4 mat1(11, 12, 13, 14,
                     21, 22, 23, 24,
                     31, 32, 33, 34,
                     41, 42, 43, 44);
    math_Mat4x4 mat2(51, 52, 53, 54,
                     61, 62, 63, 64,
                     71, 72, 73, 74,
                     81, 82, 83, 84);
    // clang-format on
    EXPECT_EQ(mat1, mat1);
    EXPECT_EQ(mat2, mat2);
    EXPECT_NE(mat1, mat2);
}

TEST(math_Mat4x4, Addition)
{
    // clang-format off
    math_Mat4x4 mat1(11, 12, 13, 14,
                     21, 22, 23, 24,
                     31, 32, 33, 34,
                     41, 42, 43, 44);
    math_Mat4x4 mat2(51, 52, 53, 54,
                     61, 62, 63, 64,
                     71, 72, 73, 74,
                     81, 82, 83, 84);
    math_Mat4x4 expected(11+51, 12+52, 13+53, 14+54,
                         21+61, 22+62, 23+63, 24+64,
                         31+71, 32+72, 33+73, 34+74,
                         41+81, 42+82, 43+83, 44+84);
    // clang-format on
    math_Mat4x4 result = mat1 + mat2;
    EXPECT_EQ(result, expected);

    mat1 += mat2;
    EXPECT_EQ(mat1, expected);
}

TEST(math_Mat4x4, Subtraction)
{
    // clang-format off
    math_Mat4x4 mat1(11, 12, 13, 14,
                     21, 22, 23, 24,
                     31, 32, 33, 34,
                     41, 42, 43, 44);
    math_Mat4x4 mat2(51, 52, 53, 54,
                     61, 62, 63, 64,
                     71, 72, 73, 74,
                     81, 82, 83, 84);
    math_Mat4x4 expected(11-51, 12-52, 13-53, 14-54,
                         21-61, 22-62, 23-63, 24-64,
                         31-71, 32-72, 33-73, 34-74,
                         41-81, 42-82, 43-83, 44-84);
    // clang-format on
    math_Mat4x4 result = mat1 - mat2;
    EXPECT_EQ(result, expected);

    mat1 -= mat2;
    EXPECT_EQ(mat1, expected);
}

TEST(math_Mat4x4, MultiplyScalar)
{
    // clang-format off
    math_Mat4x4 mat1(11, 12, 13, 14,
                     21, 22, 23, 24,
                     31, 32, 33, 34,
                     41, 42, 43, 44);
    // clang-format on
    float       factor   = 2.345f;
    math_Mat4x4 expected = mat1;
    for (float & value : expected.values)
        value *= factor;

    math_Mat4x4 result1 = mat1 * factor;
    math_Mat4x4 result2 = factor * mat1;
    EXPECT_EQ(result1, expected);
    EXPECT_EQ(result2, expected);

    mat1 *= factor;
    EXPECT_EQ(mat1, expected);
}

TEST(math_Mat4x4, MultiplyMatrix) {
    // clang-format off
    math_Mat4x4 mat1(1, 1, 1, 1,
                     2, 2, 2, 2,
                     3, 3, 3, 3,
                     4, 4, 4, 4);
    math_Mat4x4 mat2(1, 2, 3, 4,
                     1, 2, 3, 4,
                     1, 2, 3, 4,
                     1, 2, 3, 4);
    math_Mat4x4 expected(4*1, 4*2, 4*3, 4*4,
                         8*1, 8*2, 8*3, 8*4,
                         12*1, 12*2, 12*3, 12*4,
                         16*1, 16*2, 16*3, 16*4);
    // clang-format on
    math_Mat4x4 result = mat1 * mat2;
    EXPECT_EQ(result, expected);
}

TEST(math_Mat4x4, MultiplyVector) {
    // clang-format off
    math_Mat4x4 mat1(11, 12, 13, 14,
                     21, 22, 23, 24,
                     31, 32, 33, 34,
                     41, 42, 43, 44);
    math_Vec4 vec(1, 2, 3, 4);
    math_Vec4 expected(11*1 + 12*2 + 13*3 + 14*4,
                       21*1 + 22*2 + 23*3 + 24*4,
                       31*1 + 32*2 + 33*3 + 34*4,
                       41*1 + 42*2 + 43*3 + 44*4);
    // clang-format on

    math_Vec4 result = mat1 * vec;
    EXPECT_EQ(expected, result);
}

TEST(math_Mat4x4, DivideScalar)
{
    // clang-format off
    math_Mat4x4 mat1(11, 12, 13, 14,
                     21, 22, 23, 24,
                     31, 32, 33, 34,
                     41, 42, 43, 44);
    // clang-format on
    float       factor   = 2.345f;
    math_Mat4x4 expected = mat1;
    for (float & value : expected.values)
        value /= factor;

    math_Mat4x4 result1 = mat1 / factor;
    EXPECT_EQ(result1, expected);

    mat1 /= factor;
    EXPECT_EQ(mat1, expected);
}

TEST(math_Mat4x4, LookAt) {}

TEST(math_Mat4x4, Perspective) {}

TEST(math_Mat4x4, Translation) {
    math_Vec3 initialVec(1, 2, 3);
    math_Vec3 trans(5, 6, 7);
    math_Vec3 expected = initialVec + trans;

    math_Mat4x4 transMat = math_CreateTranslationMatrix(trans);
    math_Vec3 result = (transMat * math_Vec4(initialVec, 1)).xyz;
    EXPECT_EQ(result, expected);
}

TEST(math_Mat4x4, Rotation) {
    math_Vec3 initialVec(1, 0, 0);
    math_Quat rot90Y = math_QuatFromAxisAngle(math_Vec3(0, 1, 0), 90);
    math_Mat4x4 rotMat = math_CreateRotationMatrix(rot90Y);
    math_Vec3 expected(0, 0, -1);
    math_Vec3 result = (rotMat * math_Vec4(initialVec, 1)).xyz;
    EXPECT_EQ(result, expected);
}

TEST(math_Mat4x4, Scaling) {
    math_Vec3 initialVec(3, 2, 1);
    math_Vec3 scale(1, 2, -3);
    math_Mat4x4 scaleMat = math_CreateScaleMatrix(scale);
    math_Vec3 expected(3, 4, -3);
    math_Vec3 result = (scaleMat * math_Vec4(initialVec, 1)).xyz;
    EXPECT_EQ(result, expected);
}
