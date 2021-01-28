#include <gtest/gtest.h>
#include <math_vec2.h>

using namespace pge;

TEST(math_Vec2, Constants)
{
    math_Vec2 vec1 = math_Vec2::Zero();
    EXPECT_EQ(vec1.x, 0);
    EXPECT_EQ(vec1.y, 0);

    math_Vec2 vec2 = math_Vec2::One();
    EXPECT_EQ(vec2.x, 1);
    EXPECT_EQ(vec2.y, 1);
}

TEST(math_Vec2, Integrity)
{
    math_Vec2 vec1(1.234f, 5.678f);
    EXPECT_EQ(vec1.x, vec1[0]);
    EXPECT_EQ(vec1.x, vec1.xy[0]);
    EXPECT_EQ(vec1.y, vec1[1]);
    EXPECT_EQ(vec1.y, vec1.xy[1]);
}

TEST(math_Vec2, Equality)
{
    math_Vec2 vec1(1.234f, 5.678f);
    math_Vec2 vec2(4.321f, 8.765f);
    EXPECT_EQ(vec1, vec1);
    EXPECT_EQ(vec2, vec2);
    EXPECT_NE(vec1, vec2);
}

TEST(math_Vec2, Negate)
{
    math_Vec2 vec1(1.234f, 5.678f);
    EXPECT_EQ(-vec1, math_Vec2(-vec1.x, -vec1.y));
}

TEST(math_Vec2, Addition)
{
    math_Vec2 vec1(1.234f, 5.678f);
    math_Vec2 vec2(4.321f, 8.765f);
    math_Vec2 result = vec1 + vec2;
    math_Vec2 expected(vec1.x + vec2.x, vec1.y + vec2.y);
    EXPECT_EQ(result, expected);

    vec1 += vec2;
    EXPECT_EQ(vec1, expected);
}

TEST(math_Vec2, Subtraction)
{
    math_Vec2 vec1(1.234f, 5.678f);
    math_Vec2 vec2(4.321f, 8.765f);
    math_Vec2 result = vec1 - vec2;
    math_Vec2 expected(vec1.x - vec2.x, vec1.x - vec2.x);
    EXPECT_EQ(result, expected);

    vec1 -= vec2;
    EXPECT_EQ(vec1, expected);
}

TEST(math_Vec2, Multiplication)
{
    math_Vec2 vec1(1.234f, 5.678f);
    float     factor  = 1109.1995f;
    math_Vec2 result1 = vec1 * factor;
    math_Vec2 result2 = factor * vec1;
    math_Vec2 expected(vec1.x * factor, vec1.y * factor);
    EXPECT_EQ(result1, expected);
    EXPECT_EQ(result2, expected);

    vec1 *= factor;
    EXPECT_EQ(vec1, expected);
}

TEST(math_Vec2, Division)
{
    math_Vec2 vec1(1.234f, 5.678f);
    float     factor = 1109.1995f;
    math_Vec2 result = vec1 / factor;
    math_Vec2 expected(vec1.x / factor, vec1.y / factor);
    EXPECT_EQ(result, expected);

    vec1 /= factor;
    EXPECT_EQ(vec1, expected);
}

TEST(math_Vec2, Dot)
{
    math_Vec2 vec1(1.234f, 5.678f);
    math_Vec2 vec2(4.321f, 8.765f);
    float     result   = math_Dot(vec1, vec2);
    float     expected = vec1.x * vec2.x + vec1.y * vec2.y;
    EXPECT_EQ(result, expected);
}

TEST(math_Vec2, Length)
{
    math_Vec2 vec1(5.0f, 0.0f);
    float     result1 = math_Length(vec1);
    EXPECT_EQ(result1, 5.0f);
    EXPECT_EQ(result1 * result1, math_LengthSquared(vec1));

    math_Vec2 vec2(3.0f, -2.0f);
    float     result2 = math_Length(vec2);
    EXPECT_FLOAT_EQ(result2, sqrtf(13.0f));
    EXPECT_FLOAT_EQ(result2 * result2, math_LengthSquared(vec2));
}

TEST(math_Vec2, Normalize)
{
    math_Vec2 vec1(1.234f, -5.678f);
    float     vec1l = math_Length(vec1);
    math_Vec2 vec1n = math_Normalize(vec1);
    EXPECT_FLOAT_EQ(math_Length(vec1n), 1.0f);
    EXPECT_EQ(vec1, vec1n * vec1l);
}