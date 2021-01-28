#include <gtest/gtest.h>
#include <math_vec4.h>

using namespace pge;

TEST(math_Vec4, Constants)
{
    math_Vec4 vec1 = math_Vec4::Zero();
    EXPECT_EQ(vec1.x, 0);
    EXPECT_EQ(vec1.y, 0);
    EXPECT_EQ(vec1.z, 0);
    EXPECT_EQ(vec1.w, 0);
}

TEST(math_Vec4, Integrity)
{
    math_Vec4 vec1(1.234f, 5.678f, 9.101f, 3.528f);

    EXPECT_EQ(vec1.x, vec1[0]);
    EXPECT_EQ(vec1.x, vec1.xyzw[0]);

    EXPECT_EQ(vec1.y, vec1[1]);
    EXPECT_EQ(vec1.y, vec1.xyzw[1]);

    EXPECT_EQ(vec1.z, vec1[2]);
    EXPECT_EQ(vec1.z, vec1.xyzw[2]);

    EXPECT_EQ(vec1.w, vec1[3]);
    EXPECT_EQ(vec1.w, vec1.xyzw[3]);

    EXPECT_EQ(vec1.xyz, math_Vec3(vec1.x, vec1.y, vec1.z));
}

TEST(math_Vec4, Equality)
{
    math_Vec4 vec1(1.234f, 5.678f, 9.101f, 3.528f);
    math_Vec4 vec2(4.321f, 8.765f, 1.349f, 5.887f);
    EXPECT_EQ(vec1, vec1);
    EXPECT_EQ(vec2, vec2);
    EXPECT_NE(vec1, vec2);
}

TEST(math_Vec4, Negate)
{
    math_Vec4 vec1(1.234f, 5.678f, 9.101f, 3.528f);
    EXPECT_EQ(-vec1, math_Vec4(-vec1.x, -vec1.y, -vec1.z, -vec1.w));
}

TEST(math_Vec4, Addition)
{
    math_Vec4 vec1(1.234f, 5.678f, 9.101f, 3.528f);
    math_Vec4 vec2(4.321f, 8.765f, 1.349f, 5.887f);
    math_Vec4 result = vec1 + vec2;
    math_Vec4 expected(vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z, vec1.w + vec2.w);
    EXPECT_EQ(result, expected);

    vec1 += vec2;
    EXPECT_EQ(vec1, expected);
}

TEST(math_Vec4, Subtraction)
{
    math_Vec4 vec1(1.234f, 5.678f, 9.101f, 3.528f);
    math_Vec4 vec2(4.321f, 8.765f, 1.349f, 5.887f);
    math_Vec4 result = vec1 - vec2;
    math_Vec4 expected(vec1.x - vec2.x, vec1.y - vec2.y, vec1.z - vec2.z, vec1.w - vec2.w);
    EXPECT_EQ(result, expected);

    vec1 -= vec2;
    EXPECT_EQ(vec1, expected);
}

TEST(math_Vec4, Multiplication)
{
    math_Vec4 vec1(1.234f, 5.678f, 9.101f, 3.528f);
    float     factor  = 1109.1995f;
    math_Vec4 result1 = vec1 * factor;
    math_Vec4 result2 = factor * vec1;
    math_Vec4 expected(vec1.x * factor, vec1.y * factor, vec1.z * factor, vec1.w * factor);
    EXPECT_EQ(result1, expected);
    EXPECT_EQ(result2, expected);

    vec1 *= factor;
    EXPECT_EQ(vec1, expected);
}

TEST(math_Vec4, Division)
{
    math_Vec4 vec1(1.234f, 5.678f, 9.101f, 3.528f);
    float     factor  = 1109.1995f;
    math_Vec4 result1 = vec1 / factor;
    math_Vec4 expected(vec1.x / factor, vec1.y / factor, vec1.z / factor, vec1.w / factor);
    EXPECT_EQ(result1, expected);

    vec1 /= factor;
    EXPECT_EQ(vec1, expected);
}

TEST(math_Vec4, Dot)
{
    math_Vec4 vec1(1.234f, 5.678f, 9.101f, 3.528f);
    math_Vec4 vec2(4.321f, 8.765f, 1.349f, 5.887f);
    float     result   = math_Dot(vec1, vec2);
    float     expected = vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z + vec1.w * vec2.w;
    EXPECT_EQ(result, expected);
}

TEST(math_Vec4, Length)
{
    math_Vec4 vec1(5.0f, 0.0f, 0.0f, 0.0f);
    float     result1 = math_Length(vec1);
    EXPECT_EQ(result1, 5.0f);
    EXPECT_EQ(result1 * result1, math_LengthSquared(vec1));

    math_Vec4 vec2(3.0f, -2.0f, 1.0f, 2.0f);
    float     result2 = math_Length(vec2);
    EXPECT_FLOAT_EQ(result2, sqrtf(18.0f));
    EXPECT_FLOAT_EQ(result2 * result2, math_LengthSquared(vec2));
}

TEST(math_Vec4, Normalize)
{
    math_Vec4 vec1(1.234f, 5.678f, 9.101f, 3.528f);
    float     vec1l = math_Length(vec1);
    math_Vec4 vec1n = math_Normalize(vec1);
    EXPECT_FLOAT_EQ(math_Length(vec1n), 1.0f);
    EXPECT_EQ(vec1, vec1n * vec1l);
}