#include "../include/game_camera.h"

#include <math_raycasting.h>
#include <input_keyboard.h>
#include <input_mouse.h>
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
        SetPerspectiveFov(entity, math_DegToRad(60.0f), 16.0f / 9.0f, 0.01f, 1000.0f);
        if (m_activeCamera == game_EntityId_Invalid) {
            m_activeCamera = entity;
        }
    }

    void
    game_CameraManager::CreateCamera(const game_Entity& entity, float fov, float aspect, float nearClip, float farClip)
    {
        CreateCamera(entity);
        SetPerspectiveFov(entity, fov, aspect, nearClip, farClip);
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
    game_CameraManager::SetPerspectiveFov(const game_Entity& camera, float fov, float aspect, float nearClip, float farClip)
    {
        core_Assert(HasCamera(camera));
        m_cameras.at(camera).projectionMatrix = math_PerspectiveFovRH(fov, aspect, nearClip, farClip);
        m_cameras.at(camera).fov              = fov;
        m_cameras.at(camera).aspect           = aspect;
        m_cameras.at(camera).nearClip         = nearClip;
        m_cameras.at(camera).farClip          = farClip;
    }

    const math_Mat4x4
    game_CameraManager::GetViewMatrix(const game_Entity& camera) const
    {
        core_Assert(HasCamera(camera));
        auto        tid = m_tmanager->GetTransformId(camera);
        math_Mat4x4 viewMatrix;
        core_Verify(math_Invert(m_tmanager->GetWorld(tid), &viewMatrix));
        return viewMatrix;
    }

    const math_Mat4x4&
    game_CameraManager::GetProjectionMatrix(const game_Entity& camera) const
    {
        core_Assert(HasCamera(camera));
        return m_cameras.at(camera).projectionMatrix;
    }

    void
    game_CameraManager::GetPerspectiveFov(const game_Entity& camera, float* fov, float* aspect, float* nearClip, float* farClip) const
    {
        core_Assert(HasCamera(camera));
        const auto& cam = m_cameras.at(camera);
        *fov            = cam.fov;
        *aspect         = cam.aspect;
        *nearClip       = cam.nearClip;
        *farClip        = cam.farClip;
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
    game_CameraManager::HoverSelect(const math_Vec2&   hoverPosNorm,
                                    const math_Vec2&   rectSize,
                                    const math_Mat4x4& view,
                                    const math_Mat4x4& proj,
                                    float*             distanceOut) const
    {
        game_Entity closestEntity(game_EntityId_Invalid);
        float       closestDepth = std::numeric_limits<float>::max();

        for (const auto& kv : m_cameras) {
            const auto& camera = kv.first;
            if (m_activeCamera == camera)
                continue;

            math_Vec3 worldPos = m_tmanager->GetWorldPosition(m_tmanager->GetTransformId(camera));
            math_Vec4 viewPos  = view * math_Vec4(worldPos, 1);
            float     depth    = -viewPos.z;
            if (depth > closestDepth)
                continue;

            if (math_Raycast_IntersectsViewRect(worldPos, rectSize, hoverPosNorm, view, proj)) {
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

        math_Mat4x4 xform = m_tmanager->GetWorld(m_tmanager->GetTransformId(camera));
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
        os.write((const char*)&camera.fov, sizeof(camera.fov));
        os.write((const char*)&camera.aspect, sizeof(camera.aspect));
        os.write((const char*)&camera.nearClip, sizeof(camera.nearClip));
        os.write((const char*)&camera.farClip, sizeof(camera.farClip));
    }

    void
    game_CameraManager::InsertSerializedEntity(std::istream& is, const game_Entity& entity)
    {
        float fov, aspect, nearClip, farClip;
        is.read((char*)&fov, sizeof(fov));
        is.read((char*)&aspect, sizeof(aspect));
        is.read((char*)&nearClip, sizeof(nearClip));
        is.read((char*)&farClip, sizeof(farClip));
        CreateCamera(entity, fov, aspect, nearClip, farClip);
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


} // namespace pge