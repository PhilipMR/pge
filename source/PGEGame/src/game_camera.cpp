#include "../include/game_camera.h"

#include <math_raycasting.h>
#include <input_keyboard.h>
#include <input_mouse.h>
#include <gfx_debug_draw.h>
#include <algorithm>
#include <iostream>

namespace pge
{
    game_CameraManager::game_CameraManager(game_TransformManager* tmanager)
        : m_tmanager(tmanager)
        , m_activeCamera(game_EntityId_Invalid)
    {}

    void
    game_CameraManager::CreateCamera(const game_Entity& entity)
    {
        core_Assert(!HasCamera(entity));
        m_cameras[entity] = Camera{math_Mat4x4()};
        if (!m_tmanager->HasTransform(entity)) {
            m_tmanager->CreateTransform(entity);
        }
        game_PerspectiveInfo perspective;
        perspective.fov      = math_DegToRad(60.0f);
        perspective.aspect   = 16.0f / 9.0f;
        perspective.nearClip = 0.01f;
        perspective.farClip  = 1000.0f;

        SetPerspective(entity, perspective);
        if (m_activeCamera == game_EntityId_Invalid) {
            m_activeCamera = entity;
        }
    }

    void
    game_CameraManager::CreateCamera(const game_Entity& entity, const game_PerspectiveInfo& perspective)
    {
        CreateCamera(entity);
        SetPerspective(entity, perspective);
    }

    void
    game_CameraManager::DestroyCamera(const game_Entity& camera)
    {
        core_Assert(HasCamera(camera));
        m_cameras.erase(m_cameras.find(camera));
    }

    bool
    game_CameraManager::HasCamera(const game_Entity& entity) const
    {
        return m_cameras.find(entity) != m_cameras.end();
    }

    void
    game_CameraManager::GarbageCollect(const game_EntityManager& entityManager)
    {
        for (auto it = m_cameras.begin(); it != m_cameras.end();) {
            if (!entityManager.IsEntityAlive(it->first)) {
                it = m_cameras.erase(it);
            } else {
                ++it;
            }
        }
    }

    void
    game_CameraManager::SetPerspective(const game_Entity& camera, const game_PerspectiveInfo& perspective)
    {
        core_Assert(HasCamera(camera));
        m_cameras.at(camera).projectionMatrix = math_PerspectiveFovRH(perspective.fov, perspective.aspect, perspective.nearClip, perspective.farClip);
        m_cameras.at(camera).perspective      = perspective;
    }

    math_Mat4x4
    game_CameraManager::GetViewMatrix(const game_Entity& camera) const
    {
        core_Assert(HasCamera(camera));
        auto        tid = m_tmanager->GetTransformId(camera);
        math_Mat4x4 viewMatrix;
        core_Verify(math_Invert(m_tmanager->GetWorldMatrix(tid), &viewMatrix));
        return viewMatrix;
    }

    const math_Mat4x4&
    game_CameraManager::GetProjectionMatrix(const game_Entity& camera) const
    {
        core_Assert(HasCamera(camera));
        return m_cameras.at(camera).projectionMatrix;
    }

    const game_PerspectiveInfo&
    game_CameraManager::GetPerspective(const game_Entity& camera) const
    {
        core_Assert(HasCamera(camera));
        return m_cameras.at(camera).perspective;
    }

    void
    game_CameraManager::SetLookAt(const game_Entity& camera, const math_Vec3& position, const math_Vec3& target)
    {
        core_Assert(HasCamera(camera));
        core_Assert(m_tmanager->HasTransform(camera));

        const math_Vec3 forward    = target - position;
        math_Mat4x4     viewMatrix = math_LookAt(position, position + forward);
        math_Mat4x4     modelMatrix;
        core_Verify(math_Invert(viewMatrix, &modelMatrix));

        math_Vec3 newPos;
        math_Vec3 newScale;
        math_Quat newRot;
        math_DecomposeMatrix(modelMatrix, &newPos, &newRot, &newScale);
        m_tmanager->SetLocal(m_tmanager->GetTransformId(camera), newPos, newRot, newScale);
    }

    void
    game_CameraManager::Activate(const game_Entity& camera)
    {
        core_Assert(HasCamera(camera));
        m_activeCamera = camera;
    }

    const game_Entity&
    game_CameraManager::GetActiveCamera() const
    {
        return m_activeCamera;
    }

    game_Entity
    game_CameraManager::FindCameraAtCursor(const math_Vec2&   cursorNorm,
                                           const math_Vec2&   rectSize,
                                           const math_Mat4x4& view,
                                           const math_Mat4x4& proj,
                                           float*             distanceOut) const
    {
        game_Entity closestEntity(game_EntityId_Invalid);
        float       closestDepth = std::numeric_limits<float>::max();

        for (const auto& kv : m_cameras) {
            const auto& camera = kv.first;

            math_Vec3 worldPos = m_tmanager->GetWorldPosition(m_tmanager->GetTransformId(camera));
            math_Vec4 viewPos  = view * math_Vec4(worldPos, 1);
            float     depth    = -viewPos.z;
            if (depth > closestDepth)
                continue;

            if (math_Raycast_IntersectsViewRect(worldPos, rectSize, cursorNorm, view, proj)) {
                closestEntity = camera;
                closestDepth  = depth;
            }
        }

        if (distanceOut != nullptr)
            *distanceOut = closestDepth;
        return closestEntity;
    }

    void
    game_CameraManager::UpdateFPS(const game_Entity& camera)
    {
        float speed = 0.1f;

        if (!input_MouseButtonDown(input_MouseButton::RIGHT))
            return;

        if (input_KeyboardDown(input_KeyboardKey::SHIFT))
            speed *= 2;

        const math_Mat4x4& viewMatrix = GetViewMatrix(camera);
        const math_Mat4x4& projMatrix = GetProjectionMatrix(camera);

        math_Mat4x4 xform = m_tmanager->GetWorldMatrix(m_tmanager->GetTransformId(camera));
        // core_Verify(math_Invert(viewMatrix, &xform));
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
            const float dyaw     = -rotation.x * rotSpeed;
            const float dpitch   = -rotation.y * rotSpeed;

            yaw += dyaw;
            pitch += dpitch;

            forward = math_Vec3(sinf(yaw) * cosf(pitch), -cosf(yaw) * cosf(pitch), sinf(pitch));
        }

        if (math_LengthSquared(velocity) + math_LengthSquared(rotation) > 0) {
            SetLookAt(camera, position, position + forward);
        }
    }

    void
    game_CameraManager::SerializeEntity(std::ostream& os, const game_Entity& entity) const
    {
        if (!HasCamera(entity))
            return;
        const Camera& camera = m_cameras.at(entity);
        os.write((const char*)&camera.perspective, sizeof(camera.perspective));
    }

    void
    game_CameraManager::InsertSerializedEntity(std::istream& is, const game_Entity& entity)
    {
        game_PerspectiveInfo perspective;
        is.read((char*)&perspective, sizeof(perspective));
        CreateCamera(entity, perspective);
    }

    std::ostream&
    operator<<(std::ostream& os, const game_CameraManager& cm)
    {
        auto numCameras = static_cast<unsigned>(cm.m_cameras.size());
        os.write((const char*)&numCameras, sizeof(numCameras));
        for (const auto& kv : cm.m_cameras) {
            os.write((const char*)&kv.first, sizeof(kv.first));
            cm.SerializeEntity(os, kv.first);
        }
        return os;
    }

    std::istream&
    operator>>(std::istream& is, game_CameraManager& cm)
    {
        unsigned numCameras;
        is.read((char*)&numCameras, sizeof(numCameras));
        for (unsigned i = 0; i < numCameras; ++i) {
            game_Entity entity;
            is.read((char*)&entity, sizeof(entity));
            cm.InsertSerializedEntity(is, entity);
        }
        return is;
    }


    void
    game_DebugDraw_Frustum(const game_PerspectiveInfo& perspective, const math_Vec3& position, const math_Mat4x4& modelMatrix, const math_Vec3& color)
    {
        const math_Vec3 camRight = modelMatrix.Right();
        const math_Vec3 camUp    = modelMatrix.Up();
        const math_Vec3 camFwd   = modelMatrix.Forward();

        // The edges
        {
            const float     hh     = tanf(perspective.fov / 2) * perspective.farClip;
            const float     hw     = hh * perspective.aspect;
            const math_Vec3 center = position + perspective.farClip * camFwd;

            const math_Vec3 nw = center - (camRight * hw) + (camUp * hh);
            const math_Vec3 ne = center + (camRight * hw) + (camUp * hh);
            const math_Vec3 se = center + (camRight * hw) - (camUp * hh);
            const math_Vec3 sw = center - (camRight * hw) - (camUp * hh);

            gfx_DebugDraw_Line(position, nw, color);
            gfx_DebugDraw_Line(position, ne, color);
            gfx_DebugDraw_Line(position, se, color);
            gfx_DebugDraw_Line(position, sw, color);
        }

        // Near rect
        {
            const float hh = tanf(perspective.fov / 2) * perspective.nearClip;
            const float hw = hh * perspective.aspect;

            const math_Vec3 center = position + perspective.nearClip * camFwd;
            const math_Vec3 nw     = center - (camRight * hw) + (camUp * hh);
            const math_Vec3 ne     = center + (camRight * hw) + (camUp * hh);
            const math_Vec3 se     = center + (camRight * hw) - (camUp * hh);
            const math_Vec3 sw     = center - (camRight * hw) - (camUp * hh);

            gfx_DebugDraw_Line(nw, ne, color);
            gfx_DebugDraw_Line(nw, sw, color);
            gfx_DebugDraw_Line(se, sw, color);
            gfx_DebugDraw_Line(se, ne, color);
        }

        // Far rect
        {
            const float hh = tanf(perspective.fov / 2) * perspective.farClip;
            const float hw = hh * perspective.aspect;

            const math_Vec3 center = position + perspective.farClip * camFwd;
            const math_Vec3 nw     = center - (camRight * hw) + (camUp * hh);
            const math_Vec3 ne     = center + (camRight * hw) + (camUp * hh);
            const math_Vec3 se     = center + (camRight * hw) - (camUp * hh);
            const math_Vec3 sw     = center - (camRight * hw) - (camUp * hh);

            gfx_DebugDraw_Line(nw, ne, color);
            gfx_DebugDraw_Line(nw, sw, color);
            gfx_DebugDraw_Line(se, sw, color);
            gfx_DebugDraw_Line(se, ne, color);
        }
    }

} // namespace pge