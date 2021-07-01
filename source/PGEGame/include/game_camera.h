#ifndef PGE_GAME_GAME_CAMERA_H
#define PGE_GAME_GAME_CAMERA_H

#include "game_entity.h"
#include "game_transform.h"
#include <math_mat4x4.h>
#include <math_vec2.h>
#include <unordered_map>

namespace pge
{
    struct game_PerspectiveInfo {
        float fov;
        float aspect;
        float nearClip;
        float farClip;
    };

    class game_CameraManager {
        struct Camera {
            math_Mat4x4          projectionMatrix;
            game_PerspectiveInfo perspective;
        };
        std::unordered_map<game_Entity, Camera> m_cameras;
        game_TransformManager*                  m_tmanager;
        game_Entity                             m_activeCamera;

    public:
        game_CameraManager(game_TransformManager* tmanager);

        void CreateCamera(const game_Entity& entity);
        void CreateCamera(const game_Entity& entity, const game_PerspectiveInfo& perspective);

        void DestroyCamera(const game_Entity& camera);
        bool HasCamera(const game_Entity& entity) const;
        void GarbageCollect(const game_EntityManager& entityManager);

        void                 SetPerspectiveFov(const game_Entity& camera, const game_PerspectiveInfo& perspective);
        const math_Mat4x4    GetViewMatrix(const game_Entity& camera) const;
        const math_Mat4x4&   GetProjectionMatrix(const game_Entity& camera) const;
        game_PerspectiveInfo GetPerspectiveFov(const game_Entity& camera) const;
        void                 SetLookAt(const game_Entity& camera, const math_Vec3& position, const math_Vec3& target);

        void               Activate(const game_Entity& camera);
        const game_Entity& GetActiveCamera() const;

        game_Entity FindEntityAtCursor(const math_Vec2&   cursorNorm,
                                       const math_Vec2&   rectSize,
                                       const math_Mat4x4& view,
                                       const math_Mat4x4& proj,
                                       float*             distanceOut) const;

        void UpdateFPS(const game_Entity& camera);

        void SerializeEntity(std::ostream& os, const game_Entity& entity) const;
        void InsertSerializedEntity(std::istream& is, const game_Entity& entity);

        friend std::ostream& operator<<(std::ostream& os, const game_CameraManager& cm);
        friend std::istream& operator>>(std::istream& is, game_CameraManager& cm);
    };

    void game_DebugDraw_Frustum(const game_PerspectiveInfo& perspective,
                                const math_Vec3&            position,
                                const math_Mat4x4&          modelMatrix,
                                const math_Vec3&            color = math_Vec3(1.f, 1.f, 1.f));

} // namespace pge

#endif