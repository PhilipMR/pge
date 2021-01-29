#ifndef PGE_MATH_MATH_MATRIX4X4_H
#define PGE_MATH_MATH_MATRIX4X4_H

#include "math_vec4.h"
#include "math_quat.h"
#include "math_constants.h"

namespace pge
{
    struct math_Mat4x4 {
        union {
            struct {
                // clang-format off
                float
                    m11, m12, m13, m14,
                    m21, m22, m23, m24,
                    m31, m32, m33, m34,
                    m41, m42, m43, m44;
                // clang-format on
            };
            math_Vec4 rows[4];
            float     values[4 * 4];
        };

        constexpr static math_Mat4x4
        Identity()
        {
            // clang-format off
            return math_Mat4x4(
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            );
            // clang-format on
        }

        constexpr math_Mat4x4()
            : m11(1)
            , m12(0)
            , m13(0)
            , m14(0)
            , m21(0)
            , m22(1)
            , m23(0)
            , m24(0)
            , m31(0)
            , m32(0)
            , m33(1)
            , m34(0)
            , m41(0)
            , m42(0)
            , m43(0)
            , m44(1)
        {}

        constexpr math_Mat4x4(
            // clang-format off
            float m11, float m12, float m13, float m14,
            float m21, float m22, float m23, float m24,
            float m31, float m32, float m33, float m34,
            float m41, float m42, float m43, float m44
            )
            : m11(m11), m12(m12), m13(m13), m14(m14)
            , m21(m21), m22(m22), m23(m23), m24(m24)
            , m31(m31), m32(m32), m33(m33), m34(m34)
            , m41(m41), m42(m42), m43(m43), m44(m44)
        // clang-format on
        {}

        constexpr math_Vec4&
        operator[](size_t index)
        {
            return rows[index];
        }

        constexpr const math_Vec4&
        operator[](size_t index) const
        {
            return rows[index];
        }
    };

    constexpr bool
    operator==(const math_Mat4x4& lhs, const math_Mat4x4& rhs)
    {
        for (size_t row = 0; row < 4; ++row) {
            for (size_t col = 0; col < 4; ++col) {
                if (!math_FloatEqual(lhs[row][col], rhs[row][col]))
                    return false;
            }
        }
        return true;
    }

    constexpr bool
    operator!=(const math_Mat4x4& lhs, const math_Mat4x4& rhs)
    {
        for (size_t row = 0; row < 4; ++row) {
            for (size_t col = 0; col < 4; ++col) {
                if (!math_FloatEqual(lhs[row][col], rhs[row][col]))
                    return true;
            }
        }
        return false;
    }

    constexpr math_Mat4x4
    operator+(const math_Mat4x4& lhs, const math_Mat4x4& rhs)
    {
        math_Mat4x4 result;
        for (size_t row = 0; row < 4; ++row) {
            for (size_t col = 0; col < 4; ++col) {
                result[row][col] = lhs[row][col] + rhs[row][col];
            }
        }
        return result;
    }

    constexpr math_Mat4x4
    operator-(const math_Mat4x4& lhs, const math_Mat4x4& rhs)
    {
        math_Mat4x4 result;
        for (size_t row = 0; row < 4; ++row) {
            for (size_t col = 0; col < 4; ++col) {
                result[row][col] = lhs[row][col] - rhs[row][col];
            }
        }
        return result;
    }

    constexpr math_Mat4x4
    operator*(const math_Mat4x4& mat, float scalar)
    {
        math_Mat4x4 result;
        for (size_t row = 0; row < 4; ++row) {
            for (size_t col = 0; col < 4; ++col) {
                result[row][col] = mat[row][col] * scalar;
            }
        }
        return result;
    }

    constexpr math_Vec4
    operator*(const math_Mat4x4& mat, const math_Vec4& vec)
    {
        return math_Vec4(math_Dot(mat.rows[0], vec), math_Dot(mat.rows[1], vec), math_Dot(mat.rows[2], vec), math_Dot(mat.rows[3], vec));
    }

    constexpr math_Mat4x4
    operator*(float scalar, const math_Mat4x4& mat)
    {
        return mat * scalar;
    }

    constexpr math_Mat4x4
    operator/(const math_Mat4x4& mat, float scalar)
    {
        math_Mat4x4 result;
        for (size_t row = 0; row < 4; ++row) {
            for (size_t col = 0; col < 4; ++col) {
                result[row][col] = mat[row][col] / scalar;
            }
        }
        return result;
    }

    constexpr math_Mat4x4&
    operator+=(math_Mat4x4& lhs, const math_Mat4x4& rhs)
    {
        lhs = lhs + rhs;
        return lhs;
    }

    constexpr math_Mat4x4&
    operator-=(math_Mat4x4& lhs, const math_Mat4x4& rhs)
    {
        lhs = lhs - rhs;
        return lhs;
    }

    constexpr math_Mat4x4&
    operator*=(math_Mat4x4& mat, float scalar)
    {
        mat = mat * scalar;
        return mat;
    }

    constexpr math_Mat4x4&
    operator/=(math_Mat4x4& mat, float scalar)
    {
        mat = mat / scalar;
        return mat;
    }

    constexpr math_Mat4x4
    math_Transpose(const math_Mat4x4& mat)
    {
        // clang-format off
        return math_Mat4x4(
            mat.m11, mat.m21, mat.m31, mat.m41,
            mat.m12, mat.m22, mat.m32, mat.m42,
            mat.m13, mat.m23, mat.m33, mat.m43,
            mat.m14, mat.m24, mat.m34, mat.m44
        );
        // clang-format on
    }

    constexpr math_Mat4x4
    operator*(const math_Mat4x4& left, const math_Mat4x4& right)
    {
        math_Mat4x4       result;
        const math_Mat4x4 rightTransposed = math_Transpose(right);
        for (size_t row = 0; row < 4; ++row) {
            for (size_t col = 0; col < 4; ++col) {
                math_Vec4 colVec = rightTransposed[col];
                result[row][col] = math_Dot(left[row], colVec);
            }
        }
        return result;
    }

    inline math_Mat4x4
    math_LookAt(const math_Vec3& eye, const math_Vec3& target, const math_Vec3& up)
    {
        math_Vec3 forward = math_Normalize(eye - target);
        math_Vec3 right   = math_Normalize(math_Cross(up, forward));
        // clang-format off
        return math_Mat4x4(
            right.x,    right.y,   right.z,   -math_Dot(eye, right),
            up.x,       up.y,      up.z,      -math_Dot(eye, up),
            forward.x,  forward.y, forward.z, -math_Dot(eye, forward),
            0,          0,         0,          1
        );
        // clang-format on
    }

    inline math_Mat4x4
    math_Perspective(float fov, float aspect, float nearClip, float farClip)
    {
        float f = 1.0f / tanf(fov * 0.5f * math_PI / 180);
        // clang-format off
        return math_Mat4x4(
            f / aspect, 0, 0, 0,
            0, f, 0, 0,
            0, 0, (farClip + nearClip) / (nearClip - farClip), 2 * nearClip * farClip / (nearClip - farClip),
            0, 0, -1, 0
        );
        // clang-format on
    }

    constexpr math_Mat4x4
    math_CreateScaleMatrix(const math_Vec3& scale)
    {
        math_Mat4x4 result;
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                result[i][j] = (i == j) ? scale[i] : 0;
            }
        }
        return result;
    }

    constexpr math_Mat4x4
    math_CreateTranslationMatrix(const math_Vec3& translation)
    {
        math_Mat4x4 result;
        for (size_t i = 0; i < 4; ++i) {
            result[i][3] = translation[i];
        }
        return result;
    }

    inline math_Mat4x4
    math_CreateRotationMatrix(const math_Quat& rotation)
    {
        const math_Quat qnorm = math_Normalize(rotation);
        math_Mat4x4     mat;
        mat[0][0] = 1 - 2 * qnorm.y * qnorm.y - 2 * qnorm.z * qnorm.z;
        mat[0][1] = 2 * qnorm.x * qnorm.y - 2 * qnorm.z * qnorm.w;
        mat[0][2] = 2 * qnorm.x * qnorm.z + 2 * qnorm.y * qnorm.w;
        mat[0][3] = 0;

        mat[1][0] = 2 * qnorm.x * qnorm.y + 2 * qnorm.z * qnorm.w;
        mat[1][1] = 1 - 2 * qnorm.x * qnorm.x - 2 * qnorm.z * qnorm.z;
        mat[1][2] = 2 * qnorm.y * qnorm.z - 2 * qnorm.x * qnorm.w;
        mat[1][3] = 0;

        mat[2][0] = 2 * qnorm.x * qnorm.z - 2 * qnorm.y * qnorm.w;
        mat[2][1] = 2 * qnorm.y * qnorm.z + 2 * qnorm.x * qnorm.w;
        mat[2][2] = 1 - 2 * qnorm.x * qnorm.x - 2 * qnorm.y * qnorm.y;
        mat[2][3] = 0;

        mat[3][0] = 0;
        mat[3][1] = 0;
        mat[3][2] = 0;
        mat[3][3] = 1;
        return mat;
    }
} // namespace pge

#endif