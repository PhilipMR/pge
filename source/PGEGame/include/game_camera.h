#ifndef PGE_GAME_GAME_CAMERA_H
#define PGE_GAME_GAME_CAMERA_H

#include <math_mat4x4.h>

namespace pge
{
    class game_Camera {
        math_Mat4x4 m_projectionMatrix;
        math_Mat4x4 m_viewMatrix;

    public:
        game_Camera();
        void               SetPerspectiveFov(float fov, float aspect, float nearClip, float farClip);
        void               SetLookAt(const math_Vec3& position, const math_Vec3& target);
        const math_Mat4x4& GetViewMatrix() const;
        const math_Mat4x4& GetProjectionMatrix() const;
    };

    void game_Camera_UpdateFPS(game_Camera* camera, float speed);

} // namespace pge

#endif