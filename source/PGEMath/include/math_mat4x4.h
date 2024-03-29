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

        // clang-format off
        constexpr math_Mat4x4()
            : m11(1), m12(0), m13(0), m14(0)
            , m21(0), m22(1), m23(0), m24(0)
            , m31(0), m32(0), m33(1), m34(0)
            , m41(0), m42(0), m43(0), m44(1)
        {}
        // clang-format on

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

        constexpr const math_Vec3
        Right() const
        {
            return math_Vec3(rows[0][0], rows[1][0], rows[2][0]);
        }

        constexpr const math_Vec3
        Up() const
        {
            return math_Vec3(rows[0][1], rows[1][1], rows[2][1]);
        }

        constexpr const math_Vec3
        Forward() const
        {
            return -math_Vec3(rows[0][2], rows[1][2], rows[2][2]);
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
    math_LookAt(const math_Vec3& eye, const math_Vec3& target)
    {
        const math_Vec3 tmp(0, 0, 1);
        math_Vec3       forward = math_Normalize(eye - target);
        math_Vec3       right   = math_Normalize(math_Cross(tmp, forward));
        math_Vec3       up      = math_Cross(forward, right);
        // clang-format off
        return math_Mat4x4(
            right.x,    right.y,   right.z,   -math_Dot(eye, right),
            up.x,       up.y,      up.z,      -math_Dot(eye, up),
            forward.x,  forward.y, forward.z, -math_Dot(eye, forward),
            0,          0,         0,          1
        );
    }

    inline math_Mat4x4
    math_PerspectiveFovLH(float fov, float aspect, float nearClip, float farClip)
    {
        float yScale = 1.0f / tanf(fov / 2);
        float xScale = yScale / aspect;
        float zn     = nearClip;
        float zf     = farClip;

        // clang-format off
        return math_Mat4x4(
            xScale, 0 ,      0,           0,
            0,      yScale , 0,           0,
            0, 0,       zf/(zf-zn),  -zn*zf/(zf-zn),
            0, 0,       1,           0
        );
        // clang-format on
    }

    inline math_Mat4x4
    math_PerspectiveFovRH(float fov, float aspect, float nearClip, float farClip)
    {
        float yScale = 1.0f / tanf(fov / 2);
        float xScale = yScale / aspect;
        float zn     = nearClip;
        float zf     = farClip;
        // clang-format off
        return math_Mat4x4(
            xScale, 0 ,      0,           0,
            0,      yScale , 0,           0,
            0, 0,      -zf/(zf-zn), -zn*zf/(zf-zn),
            0, 0,      -1,           0
        );
        // clang-format on
    }

    inline math_Mat4x4
    math_OrthographicRH(float width, float height, float nearClip, float farClip)
    {
        // clang-format off
        return
        math_Mat4x4(
            2.0f/width, 0,              0,                              0,
            0,          2.0f/height,    0,                              0,
            0,          0,              1.0f/(nearClip-farClip),      -nearClip/(farClip-nearClip),
            0,          0,              0,                              1
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
        result[0][3] = translation[0];
        result[1][3] = translation[1];
        result[2][3] = translation[2];
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

    inline math_Mat4x4
    math_CreateTransformMatrix(const math_Vec3& position, const math_Quat& rotation, const math_Vec3& scale)
    {
        return math_CreateTranslationMatrix(position) * math_CreateScaleMatrix(scale) * math_CreateRotationMatrix(rotation);
    }

    inline bool
    math_Invert(const math_Mat4x4& mat, math_Mat4x4* out)
    {
        // Copied from gluInvertMatrix
        const float* m = &mat[0][0];
        float        inv[16];

        // clang-format off
        inv[0] = m[5] * m[10] * m[15] -
                 m[5] * m[11] * m[14] -
                 m[9] * m[6] * m[15] +
                 m[9] * m[7] * m[14] +
                 m[13] * m[6] * m[11] -
                 m[13] * m[7] * m[10];

        inv[4] = -m[4] * m[10] * m[15] +
                 m[4] * m[11] * m[14] +
                 m[8] * m[6] * m[15] -
                 m[8] * m[7] * m[14] -
                 m[12] * m[6] * m[11] +
                 m[12] * m[7] * m[10];

        inv[8] = m[4] * m[9] * m[15] -
                 m[4] * m[11] * m[13] -
                 m[8] * m[5] * m[15] +
                 m[8] * m[7] * m[13] +
                 m[12] * m[5] * m[11] -
                 m[12] * m[7] * m[9];

        inv[12] = -m[4] * m[9] * m[14] +
                  m[4] * m[10] * m[13] +
                  m[8] * m[5] * m[14] -
                  m[8] * m[6] * m[13] -
                  m[12] * m[5] * m[10] +
                  m[12] * m[6] * m[9];

        inv[1] = -m[1] * m[10] * m[15] +
                 m[1] * m[11] * m[14] +
                 m[9] * m[2] * m[15] -
                 m[9] * m[3] * m[14] -
                 m[13] * m[2] * m[11] +
                 m[13] * m[3] * m[10];

        inv[5] = m[0] * m[10] * m[15] -
                 m[0] * m[11] * m[14] -
                 m[8] * m[2] * m[15] +
                 m[8] * m[3] * m[14] +
                 m[12] * m[2] * m[11] -
                 m[12] * m[3] * m[10];

        inv[9] = -m[0] * m[9] * m[15] +
                 m[0] * m[11] * m[13] +
                 m[8] * m[1] * m[15] -
                 m[8] * m[3] * m[13] -
                 m[12] * m[1] * m[11] +
                 m[12] * m[3] * m[9];

        inv[13] = m[0] * m[9] * m[14] -
                  m[0] * m[10] * m[13] -
                  m[8] * m[1] * m[14] +
                  m[8] * m[2] * m[13] +
                  m[12] * m[1] * m[10] -
                  m[12] * m[2] * m[9];

        inv[2] = m[1] * m[6] * m[15] -
                 m[1] * m[7] * m[14] -
                 m[5] * m[2] * m[15] +
                 m[5] * m[3] * m[14] +
                 m[13] * m[2] * m[7] -
                 m[13] * m[3] * m[6];

        inv[6] = -m[0] * m[6] * m[15] +
                 m[0] * m[7] * m[14] +
                 m[4] * m[2] * m[15] -
                 m[4] * m[3] * m[14] -
                 m[12] * m[2] * m[7] +
                 m[12] * m[3] * m[6];

        inv[10] = m[0] * m[5] * m[15] -
                  m[0] * m[7] * m[13] -
                  m[4] * m[1] * m[15] +
                  m[4] * m[3] * m[13] +
                  m[12] * m[1] * m[7] -
                  m[12] * m[3] * m[5];

        inv[14] = -m[0] * m[5] * m[14] +
                  m[0] * m[6] * m[13] +
                  m[4] * m[1] * m[14] -
                  m[4] * m[2] * m[13] -
                  m[12] * m[1] * m[6] +
                  m[12] * m[2] * m[5];

        inv[3] = -m[1] * m[6] * m[11] +
                 m[1] * m[7] * m[10] +
                 m[5] * m[2] * m[11] -
                 m[5] * m[3] * m[10] -
                 m[9] * m[2] * m[7] +
                 m[9] * m[3] * m[6];

        inv[7] = m[0] * m[6] * m[11] -
                 m[0] * m[7] * m[10] -
                 m[4] * m[2] * m[11] +
                 m[4] * m[3] * m[10] +
                 m[8] * m[2] * m[7] -
                 m[8] * m[3] * m[6];

        inv[11] = -m[0] * m[5] * m[11] +
                  m[0] * m[7] * m[9] +
                  m[4] * m[1] * m[11] -
                  m[4] * m[3] * m[9] -
                  m[8] * m[1] * m[7] +
                  m[8] * m[3] * m[5];

        inv[15] = m[0] * m[5] * m[10] -
                  m[0] * m[6] * m[9] -
                  m[4] * m[1] * m[10] +
                  m[4] * m[2] * m[9] +
                  m[8] * m[1] * m[6] -
                  m[8] * m[2] * m[5];
        // clang-format on

        float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
        if (det == 0) {
            return false;
        }
        det = 1.0f / det;

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                (*out)[i][j] = inv[i * 4 + j] * det;
            }
        }
        return true;
    }

    inline math_Quat
    math_QuatFromMatrix(const math_Mat4x4& mat)
    {
        //        const float qw     = 0.5f * sqrtf(1 + mat[0][0] + mat[1][1] + mat[2][2]);
        //        const float qw4Inv = 1.0f / (4 * qw);
        //        // clang-format off
        //        return math_Quat(
        //            qw,
        //            qw4Inv * (mat[2][1] - mat[1][2]),
        //            qw4Inv * (mat[0][2] - mat[2][0]),
        //            qw4Inv * (mat[1][0] - mat[0][1])
        //        );
        //        // clang-format on


        const float trace = mat[0][0] + mat[1][1] + mat[2][2];
        if (trace > 0) {
            float s = sqrtf(trace + 1.0f) * 2;
            // clang-format off
            return math_Quat(0.25f * s,
                             (mat[2][1] - mat[1][2]) / s,
                             (mat[0][2] - mat[2][0]) / s,
                             (mat[1][0] - mat[0][1]) / s);
            // clang-format on
        } else {
            if (mat[0][0] > mat[1][1] && mat[0][0] > mat[2][2]) {
                float s = 2.0f * sqrtf(1.0f + mat[0][0] - mat[1][1] - mat[2][2]);
                // clang-format off
                return math_Quat((mat[2][1] - mat[1][2] ) / s,
                        0.25f * s,
                        (mat[0][1] + mat[1][0] ) / s,
                        (mat[0][2] + mat[2][0] ) / s);
                // clang-format on
            } else if (mat[1][1] > mat[2][2]) {
                float s = 2.0f * sqrtf(1.0f + mat[1][1] - mat[0][0] - mat[2][2]);
                // clang-format off
                return math_Quat((mat[0][2] - mat[2][0] ) / s,
                                (mat[0][1] + mat[1][0] ) / s,
                                0.25f * s,
                                (mat[1][2] + mat[2][1] ) / s);
                // clang-format on
            } else {
                float s = 2.0f * sqrtf(1.0f + mat[2][2] - mat[0][0] - mat[1][1]);
                // clang-format off
                return math_Quat((mat[1][0] - mat[0][1]) / s,
                                (mat[0][2] + mat[2][0]) / s,
                                (mat[1][2] + mat[2][1]) / s,
                                0.25f * s);
                // clang-format on
            }
        }
    }

    inline void
    math_DecomposeMatrix(const math_Mat4x4& matrix, math_Vec3* position, math_Quat* rotation, math_Vec3* scale)
    {
        const math_Vec3 c0(matrix[0][0], matrix[1][0], matrix[2][0]);
        const math_Vec3 c1(matrix[0][1], matrix[1][1], matrix[2][1]);
        const math_Vec3 c2(matrix[0][2], matrix[1][2], matrix[2][2]);
        const math_Vec3 c3(matrix[0][3], matrix[1][3], matrix[2][3]);
        *position = c3;

        *scale = math_Vec3(math_Length(c0), math_Length(c1), math_Length(c2));

        math_Mat4x4 rotMatrix;
        rotMatrix[0] = math_Vec4(math_Normalize(c0), 0);
        rotMatrix[1] = math_Vec4(math_Normalize(c1), 0);
        rotMatrix[2] = math_Vec4(math_Normalize(c2), 0);
        rotMatrix    = math_Transpose(rotMatrix); // Set c0,c1,c2 as columns instead of rows by transposing the matrix

        math_Vec3 cross = math_Cross(c0, c1);
        if (math_Dot(cross, c2) < 0) {
            rotMatrix *= -1.f;
            *scale *= -1.f;
        }
        *rotation = math_QuatFromMatrix(rotMatrix);
    }

} // namespace pge

#endif