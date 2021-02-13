#ifndef PGE_GAME_GAME_CAMERA_H
#define PGE_GAME_GAME_CAMERA_H

#include "game_transform.h"
#include <math_mat4x4.h>
#include <input_keyboard.h>
#include <input_mouse.h>

namespace pge
{
    class game_Camera {
        game_Transform m_transform;
        math_Mat4x4    m_projectionMatrix;

    public:
        game_Camera();
        game_Camera(float fov, float aspect, float nearClip, float farClip);

        game_Transform* GetTransform();
        math_Mat4x4     GetViewMatrix() const;
        math_Mat4x4     GetProjectionMatrix() const;
    };

    class game_FPSCamera {
        math_Mat4x4 m_projectionMatrix;
        math_Mat4x4 m_viewMatrix;

    public:
        game_FPSCamera()
        {
            const float fov      = 60.0f;
            const float aspect   = 1920.0f / 1080.0f;
            const float nearClip = 0.01f;
            const float farClip  = 100.0f;
            m_projectionMatrix   = math_Perspective(fov, aspect, nearClip, farClip);
        }

        game_FPSCamera(float fov, float aspect, float nearClip, float farClip)
        {
            m_projectionMatrix = math_Perspective(fov, aspect, nearClip, farClip);
        }

        void
        SetLookAt(const math_Vec3& position, const math_Vec3& target)
        {
            math_Vec3 forward = target - position;
            m_viewMatrix      = math_LookAt(position, position + forward);
        }

        void
        UpdateFPS(float speed)
        {
            if (!input_MouseButtonDown(input_MouseButton::RIGHT))
                return;

            math_Mat4x4 xform;
            diag_Verify(math_Invert(m_viewMatrix, &xform));
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
            float yaw   = (forward.x >= 0 ? 1 : -1) * acosf(math_Clamp(math_Dot(right, math_Vec3(1, 0, 0)), -1, 1));
            float pitch = (forward.y >= 0 ? 1 : -1) * acosf(math_Clamp(math_Dot(up, math_Vec3(0, 1, 0)), -1, 1));

            math_Vec2 rotation = input_MouseDelta();
            if (math_LengthSquared(rotation) > 0) {
                const float rotSpeed = 0.005f;
                yaw += rotation.x * rotSpeed;
                pitch -= rotation.y * rotSpeed;

                if (pitch > 0.25f * math_PI)
                    pitch = 0.25f * math_PI;
                if (pitch < -0.25f * math_PI)
                    pitch = -0.25f * math_PI;

                forward = math_Vec3(sinf(yaw) * cosf(pitch), sinf(pitch), -cosf(yaw) * cosf(pitch));
            }

            m_viewMatrix = math_LookAt(position, position + forward);
        }

        math_Mat4x4
        GetViewMatrix() const
        {
            return m_viewMatrix;
        }
        math_Mat4x4
        GetProjectionMatrix() const
        {
            return m_projectionMatrix;
        }
    };


} // namespace pge

#endif