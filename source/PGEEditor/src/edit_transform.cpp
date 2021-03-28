#include "../include/edit_transform.h"
#include <imgui/imgui.h>
#include <gfx_debug_draw.h>
#include <input_keyboard.h>
#include <input_mouse.h>

namespace pge
{
    // ---------------------------------
    // edit_CommandTranslate
    // ---------------------------------
    edit_CommandTranslate::edit_CommandTranslate(const game_Entity& entity, const math_Vec3& translation, game_TransformManager* tm)
        : m_entity(entity)
        , m_translation(translation)
        , m_tmanager(tm)
    {}

    void
    edit_CommandTranslate::Do()
    {
        auto tid = m_tmanager->GetTransformId(m_entity);
        m_tmanager->Translate(tid, m_translation);
    }

    void
    edit_CommandTranslate::Undo()
    {
        auto tid = m_tmanager->GetTransformId(m_entity);
        m_tmanager->Translate(tid, -m_translation);
    }

    std::unique_ptr<edit_Command>
    edit_CommandTranslate::Create(const game_Entity& entity, const math_Vec3& translation, game_TransformManager* tm)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandTranslate(entity, translation, tm));
    }


    // ---------------------------------
    // edit_CommandScale
    // ---------------------------------
    edit_CommandScale::edit_CommandScale(const game_Entity& entity, const math_Vec3& scale, game_TransformManager* tm)
        : m_entity(entity)
        , m_scale(scale)
        , m_tmanager(tm)
    {}

    void
    edit_CommandScale::Do()
    {
        auto tid = m_tmanager->GetTransformId(m_entity);
        m_tmanager->Scale(tid, m_scale);
    }

    void
    edit_CommandScale::Undo()
    {
        auto      tid = m_tmanager->GetTransformId(m_entity);
        math_Vec3 scale;
        for (size_t i = 0; i < 3; ++i) {
            scale[i] = 1.0f / m_scale[i];
        }
        m_tmanager->Scale(tid, scale);
    }

    std::unique_ptr<edit_Command>
    edit_CommandScale::Create(const game_Entity& entity, const math_Vec3& scale, game_TransformManager* tm)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandScale(entity, scale, tm));
    }


    // ---------------------------------
    // edit_CommandRotate
    // ---------------------------------
    edit_CommandSetRotation::edit_CommandSetRotation(const game_Entity& entity, const math_Quat& rotation, game_TransformManager* tm)
        : m_entity(entity)
        , m_rotation(rotation)
        , m_tmanager(tm)
    {
        auto tid          = tm->GetTransformId(entity);
        m_initialRotation = tm->GetLocalRotation(tid);
    }

    void
    edit_CommandSetRotation::Do()
    {
        auto tid = m_tmanager->GetTransformId(m_entity);
        m_tmanager->SetLocalRotation(tid, m_rotation);
    }

    void
    edit_CommandSetRotation::Undo()
    {
        auto tid = m_tmanager->GetTransformId(m_entity);
        m_tmanager->SetLocalRotation(tid, m_initialRotation);
    }

    std::unique_ptr<edit_Command>
    edit_CommandSetRotation::Create(const game_Entity& entity, const math_Quat& rotation, game_TransformManager* tm)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandSetRotation(entity, rotation, tm));
    }


    // ---------------------------------
    // edit_TransformEditor
    // ---------------------------------
    edit_TransformEditor::edit_TransformEditor(game_TransformManager* tm)
        : m_tmanager(tm)
    {}

    void
    edit_TransformEditor::UpdateAndDraw(const game_Entity& entity)
    {
        if (!m_tmanager->HasTransform(entity)) {
            if (ImGui::Button("Add transform")) {
                m_tmanager->CreateTransform(entity);
            } else {
                return;
            }
        }
        auto tid = m_tmanager->GetTransformId(entity);
        auto pos = m_tmanager->GetLocalPosition(tid);
        if (ImGui::DragFloat3("Position", &pos[0], 0.1f)) {
            m_tmanager->SetLocalPosition(tid, pos);
        }
        math_Vec3 scl = m_tmanager->GetLocalScale(tid);
        if (ImGui::DragFloat3("Scale", &scl[0], 0.1f)) {
            m_tmanager->SetLocalScale(tid, scl);
        }
        math_Quat rotQuat = m_tmanager->GetLocalRotation(tid);
        math_Vec3 rot     = math_EulerAnglesFromQuaternion(rotQuat);
        for (size_t i = 0; i < 3; ++i)
            rot[i] = math_RadToDeg(rot[i]);
        if (ImGui::DragFloat3("Rotation", &rot[0])) {
            for (size_t i = 0; i < 3; ++i)
                rot[i] = math_DegToRad(rot[i]);
            rotQuat = math_QuatFromEulerAngles(rot);
            m_tmanager->SetLocalRotation(tid, rotQuat);
        }
    }


    // ---------------------------------
    // edit_TranslateTool
    // ---------------------------------
    static void
    GetAxisVectors(const edit_Axis& axis, math_Vec3* vecsOut, size_t* numVecsOut)
    {
        switch (axis) {
            case edit_Axis::NONE: {
                *numVecsOut = 0;
            } break;
            case edit_Axis::X: {
                vecsOut[0]  = math_Vec3(1, 0, 0);
                *numVecsOut = 1;
            } break;
            case edit_Axis::Y: {
                vecsOut[0]  = math_Vec3(0, 1, 0);
                *numVecsOut = 1;
            } break;
            case edit_Axis::Z: {
                vecsOut[0]  = math_Vec3(0, 0, 1);
                *numVecsOut = 1;
            } break;
            case edit_Axis::XY: {
                vecsOut[0]  = math_Vec3(1, 0, 0);
                vecsOut[1]  = math_Vec3(0, 1, 0);
                *numVecsOut = 2;
            } break;
            case edit_Axis::XZ: {
                vecsOut[0]  = math_Vec3(1, 0, 0);
                vecsOut[1]  = math_Vec3(0, 0, 1);
                *numVecsOut = 2;
            } break;
            case edit_Axis::YZ: {
                vecsOut[0]  = math_Vec3(0, 1, 0);
                vecsOut[1]  = math_Vec3(0, 0, 1);
                *numVecsOut = 2;
            } break;
            case edit_Axis::XYZ: {
                vecsOut[0]  = math_Vec3(1, 0, 0);
                vecsOut[1]  = math_Vec3(0, 1, 0);
                vecsOut[2]  = math_Vec3(0, 0, 1);
                *numVecsOut = 3;
            } break;
            default: {
                diag_AssertWithReason(false, "Unhandled axis!");
            } break;
        }
    }

    static edit_Axis
    GetActiveAxis(const edit_Axis& axis)
    {
        if (input_KeyboardDown(input_KeyboardKey::SHIFT)) {
            if (input_KeyboardPressed(input_KeyboardKey::X)) {
                return edit_Axis::YZ;
            } else if (input_KeyboardPressed(input_KeyboardKey::Y)) {
                return edit_Axis::XZ;
            } else if (input_KeyboardPressed(input_KeyboardKey::Z)) {
                return edit_Axis::XY;
            }
        } else {
            if (input_KeyboardPressed(input_KeyboardKey::X)) {
                return edit_Axis::X;
            } else if (input_KeyboardPressed(input_KeyboardKey::Y)) {
                return edit_Axis::Y;
            } else if (input_KeyboardPressed(input_KeyboardKey::Z)) {
                return edit_Axis::Z;
            }
        }
        return axis;
    }

    static void
    DrawAxis(const game_TransformManager* tm, const game_Entity& entity, const edit_Axis& axis)
    {
        auto        tid        = tm->GetTransformId(entity);
        auto        world      = tm->GetWorld(tid);
        auto        pos        = math_Vec3(world[0][3], world[1][3], world[2][3]);
        const float lineLength = 10000.0f;

        math_Vec3 axisVecs[3];
        size_t    numAxisVecs = 0;
        GetAxisVectors(axis, axisVecs, &numAxisVecs);
        for (size_t i = 0; i < numAxisVecs; ++i) {
            const math_Vec3& vec = axisVecs[i];
            gfx_DebugDraw_Line(pos - vec * lineLength / 2, pos + vec * lineLength / 2, vec);
        }
    }

    static void
    TranslateEntity(game_TransformManager* tm, const game_Entity& entity, const edit_Axis& axis, const math_Mat4x4& viewProj, math_Vec2 delta)
    {
        delta.y *= -1;
        if (math_LengthSquared(delta) == 0)
            return;
        float     deltaMag = math_Length(delta);
        math_Vec2 deltaDir = math_Normalize(delta);

        auto tid   = tm->GetTransformId(entity);
        auto world = tm->GetWorld(tid);
        auto pos   = math_Vec3(world[0][3], world[1][3], world[2][3]);

        math_Vec3 axisVecs[3];
        size_t    numAxisVecs = 0;
        GetAxisVectors(axis, axisVecs, &numAxisVecs);
        for (size_t i = 0; i < numAxisVecs; ++i) {
            math_Vec4 taxis  = viewProj * math_Vec4(axisVecs[i], 0);
            auto      taxlen = math_Length(taxis);
            // diag_LogDebugf("taxlen = %f", taxlen);
            math_Vec2 axisDir  = math_Normalize(math_Vec2(taxis.x, taxis.y));
            float     stepSize = deltaMag * math_Dot(axisDir, deltaDir) * taxlen * 0.03f;
            pos += stepSize * axisVecs[i];
        }
        tm->SetLocalPosition(tid, pos);
    }

    edit_TranslateTool::edit_TranslateTool(game_TransformManager* tm)
        : m_tmanager(tm)
        , m_entity(game_EntityId_Invalid)
        , m_axis(edit_Axis::NONE)
        , m_initialPosition()
        , m_hasBegun(false)
    {}

    void
    edit_TranslateTool::BeginTranslation(const game_Entity& entity)
    {
        diag_Assert(!m_hasBegun);
        auto tid          = m_tmanager->GetTransformId(entity);
        m_initialPosition = m_tmanager->GetLocalPosition(tid);
        m_entity          = entity;
        m_axis            = edit_Axis::NONE;
        m_hasBegun        = true;
    }


    void
    edit_TranslateTool::CompleteTranslation(edit_CommandStack* cstack)
    {
        diag_Assert(m_hasBegun);
        m_hasBegun = false;

        auto      tid    = m_tmanager->GetTransformId(m_entity);
        math_Vec3 curPos = m_tmanager->GetLocalPosition(tid);
        math_Vec3 trans  = curPos - m_initialPosition;
        cstack->Add(edit_CommandTranslate::Create(m_entity, trans, m_tmanager));
    }

    void
    edit_TranslateTool::CancelTranslation()
    {
        if (!m_hasBegun)
            return;
        m_hasBegun = false;

        auto tid = m_tmanager->GetTransformId(m_entity);
        m_tmanager->SetLocalPosition(tid, m_initialPosition);
    }

    void
    edit_TranslateTool::UpdateAndDraw(const math_Mat4x4& viewProj, const math_Vec2& delta)
    {
        if (m_hasBegun) {
            m_axis = GetActiveAxis(m_axis);
            DrawAxis(m_tmanager, m_entity, m_axis);
            TranslateEntity(m_tmanager, m_entity, m_axis, viewProj, delta);
        }
    }


    // ---------------------------------
    // edit_ScalingTool
    // ---------------------------------
    static void
    ScaleEntity(game_TransformManager* tm, const game_Entity& entity, const edit_Axis& axis, const math_Mat4x4& viewProj, math_Vec2 delta)
    {
        delta.y *= -1;
        if (math_LengthSquared(delta) == 0)
            return;
        float     deltaMag = math_Length(delta);
        math_Vec2 deltaDir = math_Normalize(delta);

        auto tid = tm->GetTransformId(entity);
        auto scl = tm->GetLocalScale(tid);

        math_Vec3 axisVecs[3];
        size_t    numAxisVecs = 0;
        GetAxisVectors(axis, axisVecs, &numAxisVecs);
        for (size_t i = 0; i < numAxisVecs; ++i) {
            math_Vec4 taxis  = viewProj * math_Vec4(axisVecs[i], 0);
            auto      taxlen = math_Length(taxis);
            // diag_LogDebugf("taxlen = %f", taxlen);
            math_Vec2 axisDir  = math_Normalize(math_Vec2(taxis.x, taxis.y));
            float     stepSize = deltaMag * math_Dot(axisDir, deltaDir) * taxlen * 0.03f;
            scl += stepSize * axisVecs[i];
        }
        tm->SetLocalScale(tid, scl);
    }

    edit_ScalingTool::edit_ScalingTool(game_TransformManager* tm)
        : m_tmanager(tm)
        , m_entity(game_EntityId_Invalid)
        , m_axis(edit_Axis::NONE)
        , m_initialScale()
        , m_hasBegun(false)
    {}

    void
    edit_ScalingTool::BeginScale(const game_Entity& entity)
    {
        diag_Assert(!m_hasBegun);
        auto tid       = m_tmanager->GetTransformId(entity);
        auto local     = m_tmanager->GetLocal(tid);
        m_initialScale = math_Vec3(local[0][0], local[1][1], local[2][2]);
        m_entity       = entity;
        m_axis         = edit_Axis::XYZ;
        m_hasBegun     = true;
    }

    void
    edit_ScalingTool::CompleteScale(edit_CommandStack* cstack)
    {
        diag_Assert(m_hasBegun);
        m_hasBegun = false;

        auto      tid   = m_tmanager->GetTransformId(m_entity);
        auto      world = m_tmanager->GetWorld(tid);
        math_Vec3 curScl(world[0][0], world[1][1], world[2][2]);
        math_Vec3 scale(curScl.x / m_initialScale.x, curScl.y / m_initialScale.y, curScl.z / m_initialScale.z);
        cstack->Add(edit_CommandScale::Create(m_entity, scale, m_tmanager));
    }

    void
    edit_ScalingTool::CancelScale()
    {
        if (!m_hasBegun)
            return;
        m_hasBegun = false;

        auto tid = m_tmanager->GetTransformId(m_entity);
        m_tmanager->SetLocalScale(tid, m_initialScale);
    }

    void
    edit_ScalingTool::UpdateAndDraw(const math_Mat4x4& viewProj, const math_Vec2& delta)
    {
        if (m_hasBegun) {
            m_axis = GetActiveAxis(m_axis);
            DrawAxis(m_tmanager, m_entity, m_axis);
            ScaleEntity(m_tmanager, m_entity, m_axis, viewProj, delta);
        }
    }


    // ---------------------------------
    // edit_RotationTool
    // ---------------------------------
    static void
    RotateEntity(game_TransformManager* tm, const game_Entity& entity, const edit_Axis& axis, const math_Mat4x4& viewProj, math_Vec2 delta)
    {
        delta.y *= -1;
        if (math_LengthSquared(delta) == 0)
            return;
        float     deltaMag = math_Length(delta);
        math_Vec2 deltaDir = math_Normalize(delta);

        auto tid = tm->GetTransformId(entity);
        auto eulerRot = math_EulerAnglesFromQuaternion(tm->GetLocalRotation(tid));

        math_Vec3 axisVecs[3];
        size_t    numAxisVecs = 0;
        GetAxisVectors(axis, axisVecs, &numAxisVecs);
        for (size_t i = 0; i < numAxisVecs; ++i) {
            math_Vec4 taxis  = viewProj * math_Vec4(axisVecs[i], 0);
            auto      taxlen = math_Length(taxis);
            // diag_LogDebugf("taxlen = %f", taxlen);
            math_Vec2 axisDir  = math_Normalize(math_Vec2(taxis.x, taxis.y));
            float     stepSize = deltaMag * math_Dot(axisDir, deltaDir) * taxlen * 0.03f;

            eulerRot += stepSize * axisVecs[i];
        }
        tm->SetLocalRotation(tid, math_QuatFromEulerAngles(eulerRot));
    }
    edit_RotationTool::edit_RotationTool(game_TransformManager* tm)
        : m_tmanager(tm)
        , m_entity(game_EntityId_Invalid)
        , m_axis(edit_Axis::NONE)
        , m_initialRot()
        , m_hasBegun(false)
    {}

    void
    edit_RotationTool::BeginRotation(const game_Entity& entity)
    {
        diag_Assert(!m_hasBegun);
        auto tid     = m_tmanager->GetTransformId(entity);
        m_initialRot = m_tmanager->GetLocalRotation(tid);
        m_entity     = entity;
        m_axis       = edit_Axis::NONE;
        m_hasBegun   = true;
    }

    void
    edit_RotationTool::CompleteRotation(edit_CommandStack* cstack)
    {
        diag_Assert(m_hasBegun);
        m_hasBegun = false;
        auto      tid   = m_tmanager->GetTransformId(m_entity);
        cstack->Add(edit_CommandSetRotation::Create(m_entity, m_tmanager->GetLocalRotation(tid), m_tmanager));
    }

    void
    edit_RotationTool::CancelRotation()
    {
        if (!m_hasBegun)
            return;
        m_hasBegun = false;

        auto tid = m_tmanager->GetTransformId(m_entity);
        m_tmanager->SetLocalRotation(tid, m_initialRot);
    }

    void
    edit_RotationTool::UpdateAndDraw(const math_Mat4x4& viewProj, const math_Vec2& delta)
    {
        if (m_hasBegun) {
            m_axis = GetActiveAxis(m_axis);
            DrawAxis(m_tmanager, m_entity, m_axis);
            RotateEntity(m_tmanager, m_entity, m_axis, viewProj, delta);
        }
    }
} // namespace pge