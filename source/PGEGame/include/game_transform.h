#ifndef PGE_GAME_GAME_TRANSFORM_H
#define PGE_GAME_GAME_TRANSFORM_H

#include <math_vec3.h>
#include <math_quat.h>
#include <math_mat4x4.h>

namespace pge
{
    class game_Transform {
        math_Vec3 m_position;
        math_Vec3 m_scale;
        math_Quat m_rotation;

    public:
        game_Transform();
        game_Transform(const math_Vec3& position, const math_Vec3& scale, const math_Quat& rotation);

        void Translate(const math_Vec3& translation);
        void Scale(const math_Vec3& scale);
        void Rotate(const math_Vec3& axis, float degrees);

        math_Vec3   GetPosition() const;
        math_Vec3   GetScale() const;
        math_Quat   GetRotation() const;
        math_Mat4x4 GetModelMatrix() const;

        void SetPosition(const math_Vec3& position);
        void SetScale(const math_Vec3& scale);
        void SetRotation(const math_Quat& rotation);
    };
} // namespace pge

#endif