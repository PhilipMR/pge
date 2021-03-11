#ifndef PGE_GAME_GAME_CAMERA_H
#define PGE_GAME_GAME_CAMERA_H

#include "game_transform.h"
#include <math_mat4x4.h>
#include <input_keyboard.h>
#include <input_mouse.h>

namespace pge
{
    class game_FPSCamera {
        math_Mat4x4 m_projectionMatrix;
        math_Mat4x4 m_viewMatrix;

    public:
        game_FPSCamera()
            : game_FPSCamera(math_DegToRad(60.0f), 16.0f / 9.0f, 0.01f, 1000.0f)
        {}

        game_FPSCamera(float fov, float aspect, float nearClip, float farClip)
        {
            m_projectionMatrix = math_PerspectiveFovRH(fov, aspect, nearClip, farClip);
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

            if (input_KeyboardDown(input_KeyboardKey::LSHIFT))
                speed *= 2;

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
            math_Vec2 rotation = input_MouseDelta();
            if (math_LengthSquared(rotation) > 0) {
                float yaw   = (forward.x >= 0 ? 1 : -1) * acosf(math_Clamp(math_Dot(right, math_Vec3(-1, 0, 0)), -1, 1));
                float pitch = (forward.z >= 0 ? 1 : -1) * acosf(math_Clamp(math_Dot(up, math_Vec3(0, 0, 1)), -1, 1));

                const float rotSpeed = 0.005f;
                yaw -= rotation.x * rotSpeed;
                pitch -= rotation.y * rotSpeed;

                //                if (pitch > 0.25f * math_PI)
                //                    pitch = 0.25f * math_PI;
                //                if (pitch < -0.25f * math_PI)
                //                    pitch = -0.25f * math_PI;

                forward = math_Vec3(sinf(yaw) * cosf(pitch), -cosf(yaw) * cosf(pitch), sinf(pitch));
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