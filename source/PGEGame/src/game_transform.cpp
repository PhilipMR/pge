#include "../include/game_transform.h"

namespace pge
{
    game_Transform::game_Transform()
        : m_position()
        , m_scale(math_Vec3::One())
        , m_rotation()
    {}

    game_Transform::game_Transform(const math_Vec3& position)
        : m_position(position)
        , m_scale(math_Vec3::One())
        , m_rotation()
    {}

    game_Transform::game_Transform(const math_Vec3& position, const math_Vec3& scale, const math_Quat& rotation)
        : m_position(position)
        , m_scale(scale)
        , m_rotation(rotation)
    {}

    void
    game_Transform::Translate(const math_Vec3& translation)
    {
        m_position += translation;
    }

    void
    game_Transform::Scale(const math_Vec3& scale)
    {
        m_scale = math_Vec3(m_scale.x * scale.x, m_scale.y * scale.y, m_scale.z * scale.z);
    }

    void
    game_Transform::Rotate(const math_Vec3& axis, float degrees)
    {
        m_rotation = math_Rotate(m_rotation, axis, degrees);
    }

    math_Vec3
    game_Transform::GetPosition() const
    {
        return m_position;
    }

    math_Vec3
    game_Transform::GetScale() const
    {
        return m_scale;
    }

    math_Quat
    game_Transform::GetRotation() const
    {
        return m_rotation;
    }

    math_Mat4x4
    game_Transform::GetModelMatrix() const
    {
        return math_CreateTranslationMatrix(m_position) * math_CreateRotationMatrix(m_rotation) * math_CreateScaleMatrix(m_scale);
    }

    void
    game_Transform::SetPosition(const math_Vec3& position)
    {
        m_position = position;
    }

    void
    game_Transform::SetScale(const math_Vec3& scale)
    {
        m_scale = scale;
    }

    void
    game_Transform::SetRotation(const math_Quat& rotation)
    {
        m_rotation = rotation;
    }
} // namespace pge