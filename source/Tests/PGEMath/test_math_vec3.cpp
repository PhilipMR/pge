#include <gtest/gtest.h>
#include <math_vec3.h>

using namespace pge;

TEST(math_Vec3, Constants)
{
    math_Vec3 vec1 = math_Vec3::Zero();
    EXPECT_EQ(vec1.x, 0);
    EXPECT_EQ(vec1.y, 0);
    EXPECT_EQ(vec1.z, 0);

    math_Vec3 vec2 = math_Vec3::One();
    EXPECT_EQ(vec2.x, 1);
    EXPECT_EQ(vec2.y, 1);
    EXPECT_EQ(vec2.z, 1);
}

TEST(math_Vec3, Integrity)
{
    math_Vec3 vec1(1.234f, 5.678f, 9.101f);

    EXPECT_EQ(vec1.x, vec1[0]);
    EXPECT_EQ(vec1.x, vec1.xyz[0]);

    EXPECT_EQ(vec1.y, vec1[1]);
    EXPECT_EQ(vec1.y, vec1.xyz[1]);

    EXPECT_EQ(vec1.z, vec1[2]);
    EXPECT_EQ(vec1.z, vec1.xyz[2]);
}

TEST(math_Vec3, Equality)
{
    math_Vec3 vec1(1.234f, 5.678f, 9.101f);
    math_Vec3 vec2(4.321f, 8.765f, 1.349f);
    EXPECT_EQ(vec1, vec1);
    EXPECT_EQ(vec2, vec2);
    EXPECT_NE(vec1, vec2);
}

TEST(math_Vec3, Negate)
{
    math_Vec3 vec1(1.234f, 5.678f, 9.101f);
    EXPECT_EQ(-vec1, math_Vec3(-vec1.x, -vec1.y, -vec1.z));
}

TEST(math_Vec3, Addition)
{
    math_Vec3 vec1(1.234f, 5.678f, 9.101f);
    math_Vec3 vec2(4.321f, 8.765f, 1.349f);
    math_Vec3 result = vec1 + vec2;
    math_Vec3 expected(vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z);
    EXPECT_EQ(result, expected);

    vec1 += vec2;
    EXPECT_EQ(vec1, expected);
}

TEST(math_Vec3, Subtraction)
{
    math_Vec3 vec1(1.234f, 5.678f, 9.101f);
    math_Vec3 vec2(4.321f, 8.765f, 1.349f);
    math_Vec3 result = vec1 - vec2;
    math_Vec3 expected(vec1.x - vec2.x, vec1.y - vec2.y, vec1.z - vec2.z);
    EXPECT_EQ(result, expected);

    vec1 -= vec2;
    EXPECT_EQ(vec1, expected);
}

TEST(math_Vec3, Multiplication)
{
    math_Vec3 vec1(1.234f, 5.678f, 9.101f);
    float     factor  = 1109.1995f;
    math_Vec3 result1 = vec1 * factor;
    math_Vec3 result2 = factor * vec1;
    math_Vec3 expected(vec1.x * factor, vec1.y * factor, vec1.z * factor);
    EXPECT_EQ(result1, expected);
    EXPECT_EQ(result2, expected);

    vec1 *= factor;
    EXPECT_EQ(vec1, expected);
}

TEST(math_Vec3, Division)
{
    math_Vec3 vec1(1.234f, 5.678f, 9.101f);
    float     factor  = 1109.1995f;
    math_Vec3 result1 = vec1 / factor;
    math_Vec3 expected(vec1.x / factor, vec1.y / factor, vec1.z / factor);
    EXPECT_EQ(result1, expected);

    vec1 /= factor;
    EXPECT_EQ(vec1, expected);
}

TEST(math_Vec3, Dot)
{
    math_Vec3 vec1(1.234f, 5.678f, 9.101f);
    math_Vec3 vec2(4.321f, 8.765f, 1.349f);
    float     result   = math_Dot(vec1, vec2);
    float     expected = vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
    EXPECT_EQ(result, expected);
}

TEST(math_Vec3, Cross)
{
    math_Vec3 vec1(1.0f, 0.0f, 0.0f);
    math_Vec3 vec2(0.0f, 1.0f, 0.0f);
    math_Vec3 result = math_Cross(vec1, vec2);
    EXPECT_EQ(result, math_Vec3(0.0f, 0.0f, 1.0f));
}

TEST(math_Vec3, Length)
{
    math_Vec3 vec1(5.0f, 0.0f, 0.0f);
    float     result1 = math_Length(vec1);
    EXPECT_EQ(result1, 5.0f);
    EXPECT_EQ(result1 * result1, math_LengthSquared(vec1));

    math_Vec3 vec2(3.0f, -2.0f, 1.0f);
    float     result2 = math_Length(vec2);
    EXPECT_FLOAT_EQ(result2, sqrtf(14.0f));
    EXPECT_FLOAT_EQ(result2 * result2, math_LengthSquared(vec2));
}

TEST(math_Vec3, Normalize)
{
    math_Vec3 vec1(1.234f, 5.678f, 9.101f);
    float     vec1l = math_Length(vec1);
    math_Vec3 vec1n = math_Normalize(vec1);
    EXPECT_FLOAT_EQ(math_Length(vec1n), 1.0f);
    EXPECT_EQ(vec1, vec1n * vec1l);
}