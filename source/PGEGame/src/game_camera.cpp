#include "../include/game_camera.h"
#include <input_keyboard.h>
#include <input_mouse.h>

namespace pge
{
    game_Camera::game_Camera()
    {
        SetPerspectiveFov(math_DegToRad(60.0f), 16.0f / 9.0f, 0.01f, 1000.0f);
    }

    void
    game_Camera::SetPerspectiveFov(float fov, float aspect, float nearClip, float farClip)
    {
        m_projectionMatrix = math_PerspectiveFovRH(fov, aspect, nearClip, farClip);
    }

    void
    game_Camera::SetLookAt(const math_Vec3& position, const math_Vec3& target)
    {
        math_Vec3 forward = target - position;
        m_viewMatrix      = math_LookAt(position, position + forward);
    }

    const math_Mat4x4&
    game_Camera::GetViewMatrix() const
    {
        return m_viewMatrix;
    }

    const math_Mat4x4&
    game_Camera::GetProjectionMatrix() const
    {
        return m_projectionMatrix;
    }

    void
    game_Camera_UpdateFPS(game_Camera* camera, float speed)
    {
        if (!input_MouseButtonDown(input_MouseButton::RIGHT))
            return;

        if (input_KeyboardDown(input_KeyboardKey::SHIFT))
            speed *= 2;

        const math_Mat4x4& viewMatrix = camera->GetViewMatrix();
        const math_Mat4x4& projMatrix = camera->GetProjectionMatrix();

        math_Mat4x4 xform;
        core_Verify(math_Invert(viewMatrix, &xform));
        math_Vec3 right    = math_Vec3(xform[0][0], xform[1][0], xform[2][0]);
        math_Vec3 up       = math_Vec3(xform[0][1], xform[1][1], xform[2][1]);
        math_Vec3 forward  = -math_Vec3(xform[0][2], xform[1][2], xform[2][2]);
        math_Vec3 position = math_Vec3(xform[0][3], xform[1][3], xform[2][3]);

        // Update camera movement.
        math_Vec3 velocity;
        if (input_KeyboardDown(input_KeyboardKey::ARROW_UP) || input_KeyboardDown(input_KeyboardKey::W)) {
            velocity += forward;
        }
        if (input_KeyboardDown(input_KeyboardKey::ARROW_LEFT) || input_KeyboardDown(input_KeyboardKey::A)) {
            velocity -= right;
        }
        if (input_KeyboardDown(input_KeyboardKey::ARROW_DOWN) || input_KeyboardDown(input_KeyboardKey::S)) {
            velocity -= forward;
        }
        if (input_KeyboardDown(input_KeyboardKey::ARROW_RIGHT) || input_KeyboardDown(input_KeyboardKey::D)) {
            velocity += right;
        }
        if (math_LengthSquared(velocity) > 0) {
            position += math_Normalize(velocity) * speed;
        }

        // Update camera rotation.
        math_Vec2 rotation = input_MouseDelta();
        if (math_LengthSquared(rotation) > 0) {
            float yaw   = (forward.x >= 0.f ? 1.f : -1.f) * acosf(math_Clamp(math_Dot(right, math_Vec3(-1, 0, 0)), -1, 1));
            float pitch = (forward.z >= 0.f ? 1.f : -1.f) * acosf(math_Clamp(math_Dot(up, math_Vec3(0, 0, 1)), -1, 1));

            const float rotSpeed = 0.005f;
            yaw -= rotation.x * rotSpeed;
            pitch -= rotation.y * rotSpeed;

            forward = math_Vec3(sinf(yaw) * cosf(pitch), -cosf(yaw) * cosf(pitch), sinf(pitch));
        }

        camera->SetLookAt(position, position + forward);
    }
} // namespace pge