#ifndef PGE_GAME_GAME_CAMERA_H
#define PGE_GAME_GAME_CAMERA_H

#include "game_transform.h"
#include <math_mat4x4.h>

namespace pge
{
    class game_Camera {
        game_Transform m_transform;
        math_Mat4x4    m_projectionMatrix;

    public:
        game_Camera(float fov, float aspect, float nearClip, float farClip);

        game_Transform* GetTransform();
        math_Mat4x4     GetViewMatrix() const;
        math_Mat4x4     GetProjectionMatrix() const;
    };
} // namespace pge

#endif