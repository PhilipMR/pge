#include "../include/game_camera.h"

namespace pge
{
    game_Camera::game_Camera()
    {
        const float fov = 60.0f;
        const float aspect = 1920.0f / 1080.0f;
        const float nearClip = 0.01f;
        const float farClip = 100.0f;
        m_projectionMatrix = math_Perspective(fov, aspect, nearClip, farClip);
    }

    game_Camera::game_Camera(float fov, float aspect, float nearClip, float farClip)
    {
        m_projectionMatrix = math_Perspective(fov, aspect, nearClip, farClip);
    }

    game_Transform*
    game_Camera::GetTransform()
    {
        return &m_transform;
    }

    math_Mat4x4
    game_Camera::GetViewMatrix() const
    {
        return math_LookAt(m_transform.GetPosition(), math_Vec3());
    }

    math_Mat4x4
    game_Camera::GetProjectionMatrix() const
    {
        return m_projectionMatrix;
    }
} // namespace pge