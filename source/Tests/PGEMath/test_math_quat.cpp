#include <gtest/gtest.h>
#include <math_quat.h>

using namespace pge;

TEST(math_Quat, Integrity)
{
    math_Quat quat(1.1f, 2.2f, 3.3f, 4.4f);
    EXPECT_EQ(quat.w, quat[0]);

    EXPECT_EQ(quat.x, quat[1]);
    EXPECT_EQ(quat.x, quat.xyz.x);

    EXPECT_EQ(quat.y, quat[2]);
    EXPECT_EQ(quat.y, quat.xyz.y);

    EXPECT_EQ(quat.z, quat[3]);
    EXPECT_EQ(quat.z, quat.xyz.z);
}

TEST(math_Quat, Equality)
{
    math_Quat quat1(1.1f, 2.2f, 3.3f, 4.4f);
    math_Quat quat2(5.5f, 6.6f, 7.7f, 8.8f);
    EXPECT_EQ(quat1, quat1);
    EXPECT_EQ(quat2, quat2);
    EXPECT_NE(quat1, quat2);
}

TEST(math_Quat, Negate)
{
    math_Quat quat1(1.1f, 2.2f, 3.3f, 4.4f);
    EXPECT_EQ(-quat1, math_Quat(-quat1.w, -quat1.x, -quat1.y, -quat1.z));
}

TEST(math_Quat, Addition)
{
    math_Quat quat1(1.234f, 5.678f, 9.101f, 3.528f);
    math_Quat quat2(4.321f, 8.765f, 1.349f, 5.887f);
    math_Quat result = quat1 + quat2;
    math_Quat expected(quat1.w + quat2.w, quat1.x + quat2.x, quat1.y + quat2.y, quat1.z + quat2.z);
    EXPECT_EQ(result, expected);

    quat1 += quat2;
    EXPECT_EQ(quat1, expected);
}

TEST(math_Quat, Subtraction)
{
    math_Quat quat1(1.234f, 5.678f, 9.101f, 3.528f);
    math_Quat quat2(4.321f, 8.765f, 1.349f, 5.887f);
    math_Quat result = quat1 - quat2;
    math_Quat expected(quat1.w - quat2.w, quat1.x - quat2.x, quat1.y - quat2.y, quat1.z - quat2.z);
    EXPECT_EQ(result, expected);

    quat1 -= quat2;
    EXPECT_EQ(quat1, expected);
}

TEST(math_Quat, MultiplyScalar)
{
    math_Quat quat1(1.234f, 5.678f, 9.101f, 3.528f);
    float     factor  = 1109.1995f;
    math_Quat result1 = quat1 * factor;
    math_Quat result2 = factor * quat1;
    math_Quat expected(quat1.w * factor, quat1.x * factor, quat1.y * factor, quat1.z * factor);
    EXPECT_EQ(result1, expected);
    EXPECT_EQ(result2, expected);

    quat1 *= factor;
    EXPECT_EQ(quat1, expected);
}

TEST(math_Quat, DivideScalar)
{
    math_Quat quat1(1.234f, 5.678f, 9.101f, 3.528f);
    float     factor  = 1109.1995f;
    math_Quat result1 = quat1 / factor;
    math_Quat expected(quat1.w / factor, quat1.x / factor, quat1.y / factor, quat1.z / factor);
    EXPECT_EQ(result1, expected);

    quat1 /= factor;
    EXPECT_EQ(quat1, expected);
}

TEST(math_Quat, MultiplyQuat)
{
    math_Quat quat1(1.234f, 5.678f, 9.101f, 3.528f);
    math_Quat quat2(4.321f, 8.765f, 1.349f, 5.887f);
    math_Quat result = quat1 * quat2;
    math_Quat expected(quat2.w * quat1.w - math_Dot(quat1.xyz, quat2.xyz),
                       quat2.w * quat1.xyz + quat1.w * quat2.xyz + math_Cross(quat1.xyz, quat2.xyz));
    EXPECT_EQ(result, expected);

    quat1 *= quat2;
    EXPECT_EQ(quat1, expected);
}

TEST(math_Quat, Conjugate)
{
    math_Quat quat1(1.234f, 5.678f, 9.101f, 3.528f);
    math_Quat result = math_Conjugate(quat1);
    math_Quat expected(quat1.w, -quat1.xyz);
    EXPECT_EQ(result, expected);
}

TEST(math_Quat, Dot)
{
    math_Quat quat1(1.234f, 5.678f, 9.101f, 3.528f);
    math_Quat quat2(4.321f, 8.765f, 1.349f, 5.887f);
    float     result   = math_Dot(quat1, quat2);
    float     expected = quat1.x * quat2.x + quat1.y * quat2.y + quat1.z * quat2.z + quat1.w * quat2.w;
    EXPECT_EQ(result, expected);
}

TEST(math_Quat, Length)
{
    math_Vec4 quat1(5.0f, 0.0f, 0.0f, 0.0f);
    float     result1 = math_Length(quat1);
    EXPECT_EQ(result1, 5.0f);
    EXPECT_EQ(result1 * result1, math_LengthSquared(quat1));

    math_Vec4 quat2(3.0f, -2.0f, 1.0f, 2.0f);
    float     result2 = math_Length(quat2);
    EXPECT_FLOAT_EQ(result2, sqrtf(18.0f));
    EXPECT_FLOAT_EQ(result2 * result2, math_LengthSquared(quat2));
}

TEST(math_Quat, Normalize)
{
    math_Quat quat1(1.234f, 5.678f, 9.101f, 3.528f);
    float     quat1l = math_Length(quat1);
    math_Quat quat1n = math_Normalize(quat1);
    EXPECT_FLOAT_EQ(math_Length(quat1n), 1.0f);
    EXPECT_EQ(quat1, quat1n * quat1l);
}

TEST(math_Quat, Invert) {
    math_Quat quat1(1.234f, 5.678f, 9.101f, 3.528f);
    math_Quat quat1Inv = math_Invert(quat1);
    math_Quat result = quat1 * quat1Inv;
    EXPECT_EQ(result, math_Quat(1, 0, 0, 0));
}

TEST(math_Quat, Lerp) {
    math_Quat quat1(1.234f, 5.678f, 9.101f, 3.528f);
    math_Quat quat2(4.321f, 8.765f, 1.349f, 5.887f);
    math_Quat diff = quat2 - quat1;

    math_Quat begin = math_Lerp(quat1, quat2, 0.0f);
    math_Quat mid = math_Lerp(quat1, quat2, 0.5f);
    math_Quat end = math_Lerp(quat1, quat2, 1.0f);
    EXPECT_EQ(begin, quat1);
    EXPECT_EQ(mid, quat1 + 0.5f * diff);
    EXPECT_EQ(end, quat2);
}

TEST(math_Quat, FromAxisAngle) {
    math_Quat quat1 = math_QuaternionFromAxisAngle(math_Vec3(), 0.0f);
    EXPECT_EQ(quat1, math_Quat(1, 0, 0, 0));

    math_Quat quat2 = math_QuaternionFromAxisAngle(math_Vec3(0, 1, 0), 90.0f);
    const float halfSqrt2 = 0.5f * sqrtf(2);
    EXPECT_EQ(quat2, math_Quat(halfSqrt2, 0, halfSqrt2, 0));
}

TEST(math_Quat, RotateVec) {
    math_Vec4 vec1(1.0f, 0.0f, 0.0f, 0.0f);
    math_Quat rot90DegY = math_QuaternionFromAxisAngle(math_Vec3(0, 1, 0), 90.0f);
    math_Vec4 rotatedVec1 = math_Rotate(vec1, rot90DegY);
    EXPECT_EQ(rotatedVec1.xyz, math_Vec3(0, 0, -1.0f));

    math_Vec4 vec2(0.0f, 1.0f, 0.0f, 0.0f);
    math_Quat rot45DegX = math_QuaternionFromAxisAngle(math_Vec3(1, 0, 0), 45.0f);
    math_Vec4 rotatedVec2 = math_Rotate(vec2, rot45DegX);
    const float halfSqrt2 = 0.5f * sqrtf(2);
    EXPECT_EQ(rotatedVec2.xyz, math_Vec3(0, halfSqrt2, halfSqrt2));

    math_Vec3 vec3(0.0f, 0.0f, 1.0f);
    math_Vec3 rotatedVec3 = math_Rotate(vec3, rot90DegY);
    EXPECT_EQ(rotatedVec3, math_Vec3(1, 0, 0));
}