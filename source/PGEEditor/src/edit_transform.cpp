#include "../include/edit_transform.h"
#include <gfx_debug_draw.h>
#include <input_keyboard.h>
#include <input_mouse.h>
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>

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
    edit_CommandSetRotation::edit_CommandSetRotation(const game_Entity&     entity,
                                                     const math_Quat&       initialRot,
                                                     const math_Quat&       newRot,
                                                     game_TransformManager* tm)
        : m_entity(entity)
        , m_initialRotation(initialRot)
        , m_rotation(newRot)
        , m_tmanager(tm)
    {}

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
    edit_CommandSetRotation::Create(const game_Entity& entity, const math_Quat& initialRot, const math_Quat& newRot, game_TransformManager* tm)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandSetRotation(entity, initialRot, newRot, tm));
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
        if (!ImGui::CollapsingHeader("Transform"))
            return;

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
                core_AssertWithReason(false, "Unhandled axis!");
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
        auto        world      = tm->GetWorldMatrix(tid);
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
        auto local = tm->GetLocalMatrix(tid);
        auto pos   = math_Vec3(local[0][3], local[1][3], local[2][3]);

        math_Vec3 axisVecs[3];
        size_t    numAxisVecs = 0;
        GetAxisVectors(axis, axisVecs, &numAxisVecs);
        for (size_t i = 0; i < numAxisVecs; ++i) {
            math_Vec4 taxis    = viewProj * math_Vec4(axisVecs[i], 0);
            auto      taxlen   = math_Length(taxis);
            math_Vec2 axisDir  = math_Normalize(math_Vec2(taxis.x, taxis.y));
            float     stepSize = deltaMag * math_Dot(axisDir, deltaDir) * taxlen * 0.03f;
            pos += stepSize * axisVecs[i];
        }
        tm->SetLocalPosition(tid, pos);
    }


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
            math_Vec4 taxis    = viewProj * math_Vec4(axisVecs[i], 0);
            auto      taxlen   = math_Length(taxis);
            math_Vec2 axisDir  = math_Normalize(math_Vec2(taxis.x, taxis.y));
            float     stepSize = deltaMag * math_Dot(axisDir, deltaDir) * taxlen * 0.03f;
            scl += stepSize * axisVecs[i];
        }
        tm->SetLocalScale(tid, scl);
    }


    static void
    RotateEntity(game_TransformManager* tm, const game_Entity& entity, const edit_Axis& axis, const math_Mat4x4& viewProj, math_Vec2 delta)
    {
        delta.y *= -1;
        if (math_LengthSquared(delta) == 0)
            return;
        float     deltaMag = math_Length(delta);
        math_Vec2 deltaDir = math_Normalize(delta);

        auto tid      = tm->GetTransformId(entity);
        auto eulerRot = math_EulerAnglesFromQuaternion(tm->GetLocalRotation(tid));

        math_Vec3 axisVecs[3];
        size_t    numAxisVecs = 0;
        GetAxisVectors(axis, axisVecs, &numAxisVecs);
        for (size_t i = 0; i < numAxisVecs; ++i) {
            math_Vec4 taxis  = viewProj * math_Vec4(axisVecs[i], 0);
            auto      taxlen = math_Length(taxis);
            // core_LogDebugf("taxlen = %f", taxlen);
            math_Vec2 axisDir  = math_Normalize(math_Vec2(taxis.x, taxis.y));
            float     stepSize = deltaMag * math_Dot(axisDir, deltaDir) * taxlen * 0.03f;

            eulerRot += stepSize * axisVecs[i];
        }
        if (numAxisVecs > 0) {
            tm->SetLocalRotation(tid, math_QuatFromEulerAngles(eulerRot));
        }
    }

    edit_TransformGizmo::edit_TransformGizmo(game_TransformManager* tm, edit_CommandStack* cstack)
        : m_opMode(OPMODE_NONE)
        , m_relMode(RELMODE_WORLD)
        , m_tmanager(tm)
        , m_cstack(cstack)
        , m_axis(edit_Axis::NONE)
        , m_entity(game_EntityId_Invalid)
        , m_isManipulating(false)
    {}

    bool
    edit_TransformGizmo::IsVisible() const
    {
        return m_opMode != OPMODE_NONE && m_entity != game_EntityId_Invalid;
    }

    bool
    edit_TransformGizmo::HasBegun() const
    {
        return m_entity != game_EntityId_Invalid;
    }

    void
    edit_TransformGizmo::Begin(const OpMode& opMode, const RelMode& relMode, const game_Entity& entity)
    {
        core_Assert(!HasBegun());
        m_opMode  = opMode;
        m_relMode = relMode;
        m_entity  = entity;

        auto tid           = m_tmanager->GetTransformId(m_entity);
        m_initial.position = m_tmanager->GetLocalPosition(tid);
        m_initial.scale    = m_tmanager->GetLocalScale(tid);
        m_initial.rotation = m_tmanager->GetLocalRotation(tid);
    }

    void
    edit_TransformGizmo::Cancel()
    {
        if (!HasBegun())
            return;

        auto tid = m_tmanager->GetTransformId(m_entity);
        m_tmanager->SetLocalPosition(tid, m_initial.position);
        m_tmanager->SetLocalScale(tid, m_initial.scale);
        m_tmanager->SetLocalRotation(tid, m_initial.rotation);

        m_opMode = OPMODE_NONE;
        m_entity = game_EntityId_Invalid;
    }


    void
    edit_TransformGizmo::Complete()
    {
        core_Assert(HasBegun());
        core_Assert(m_opMode != OPMODE_NONE);

        auto tid = m_tmanager->GetTransformId(m_entity);
        switch (m_opMode) {
            case OPMODE_TRANSLATE: {
                m_cstack->Add(edit_CommandTranslate::Create(m_entity, m_tmanager->GetLocalPosition(tid) - m_initial.position, m_tmanager));
            } break;
            case OPMODE_SCALE: {
                math_Vec3 localScl = m_tmanager->GetLocalScale(tid);
                math_Vec3 scl;
                for (size_t i = 0; i < 3; ++i)
                    scl[i] = localScl[i] / m_initial.scale[i];
                m_cstack->Add(edit_CommandScale::Create(m_entity, scl, m_tmanager));
            }; break;
            case OPMODE_ROTATE: {
                m_cstack->Add(edit_CommandSetRotation::Create(m_entity, m_initial.rotation, m_tmanager->GetLocalRotation(tid), m_tmanager));
            } break;
        }

        m_opMode = OPMODE_NONE;
        m_entity = game_EntityId_Invalid;
    }

    void
    edit_TransformGizmo::TransformEntity(const game_Entity& entity, const math_Mat4x4& view, const math_Mat4x4& proj)
    {
        // Change transform to another entity
        if (HasBegun() && entity != game_EntityId_Invalid && entity != m_entity) {
            Complete();
        }
        // Stop transform (select elsewhere/non-transform entity)
        if (entity == game_EntityId_Invalid || !m_tmanager->HasTransform(entity)) {
            if (HasBegun()) {
                Complete();
            }
            return;
        }

        // Handle mode transition
        if (input_KeyboardPressed(input_KeyboardKey::G)) {
            // If already in operation, 'G' toggles relative mode
            if (m_opMode == OPMODE_TRANSLATE) {
                m_relMode = m_relMode != RELMODE_LOCAL ? RELMODE_LOCAL : RELMODE_WORLD;
            }
            Cancel();
            Begin(OPMODE_TRANSLATE, m_relMode, entity);
            m_axis = edit_Axis::NONE;
        }

        if (input_KeyboardPressed(input_KeyboardKey::S) && !input_MouseButtonDown(input_MouseButton::RIGHT)) {
            // If already in operation, 'S' toggles relative mode
            if (m_opMode == OPMODE_SCALE) {
                m_relMode = m_relMode != RELMODE_LOCAL ? RELMODE_LOCAL : RELMODE_WORLD;
            }
            Cancel();
            Begin(OPMODE_SCALE, m_relMode, entity);
            m_axis = edit_Axis::NONE;
        }

        if (input_KeyboardPressed(input_KeyboardKey::R)) {
            // If already in operation, 'R' toggles relative mode
            if (m_opMode == OPMODE_ROTATE) {
                m_relMode = m_relMode != RELMODE_LOCAL ? RELMODE_LOCAL : RELMODE_WORLD;
            }
            Cancel();
            Begin(OPMODE_ROTATE, m_relMode, entity);
            m_axis = edit_Axis::NONE;
        }

        // Update and draw gizmo
        if (m_opMode != OPMODE_NONE) {
            game_TransformId          tid = m_tmanager->GetTransformId(entity);
            const ImGuizmo::OPERATION guizmoOp
                = (m_opMode == OPMODE_TRANSLATE) ? (ImGuizmo::TRANSLATE) : (m_opMode == OPMODE_ROTATE ? (ImGuizmo::ROTATE) : (ImGuizmo::SCALE));
            const ImGuizmo::MODE guizmoMode = m_relMode == RELMODE_LOCAL ? ImGuizmo::LOCAL : ImGuizmo::WORLD;

            math_Mat4x4       localT = math_Transpose(m_tmanager->GetWorldMatrix(tid));
            const math_Mat4x4 viewT  = math_Transpose(view);
            const math_Mat4x4 projT  = math_Transpose(proj);
            if (ImGuizmo::Manipulate(&viewT[0][0], &projT[0][0], guizmoOp, guizmoMode, &localT[0][0], nullptr, nullptr)) {
                m_isManipulating = true;

                auto        parent = m_tmanager->GetParent(tid);
                math_Mat4x4 parentInv;
                if (parent != game_TransformId_Invalid) {
                    core_Verify(math_Invert(m_tmanager->GetWorldMatrix(parent), &parentInv));
                    parentInv = math_Transpose(parentInv);
                }
                localT = localT * parentInv;

                math_Vec3 newRotEul, newPos, newScale;
                ImGuizmo::DecomposeMatrixToComponents(&localT[0][0], &newPos[0], &newRotEul[0], &newScale[0]);

                newRotEul *= math_PI / 180.0f;

                m_tmanager->SetLocalPosition(tid, newPos);
                m_tmanager->SetLocalRotation(tid, math_QuatFromEulerAngles(newRotEul));
                m_tmanager->SetLocalScale(tid, newScale);
            } else {
                if (m_isManipulating) {
                    const OpMode opMode = m_opMode;
                    Complete();
                    Begin(opMode, m_relMode, entity);
                    m_isManipulating = false;
                }
                if (HasBegun()) {
                    m_axis = GetActiveAxis(m_axis);
                    DrawAxis(m_tmanager, entity, m_axis);
                    const math_Mat4x4 viewProj = proj * view;
                    const math_Vec2   delta    = input_MouseDelta();
                    switch (m_opMode) {
                        case OPMODE_TRANSLATE: {
                            TranslateEntity(m_tmanager, entity, m_axis, viewProj, delta);
                        } break;
                        case OPMODE_ROTATE: {
                            RotateEntity(m_tmanager, entity, m_axis, viewProj, delta);
                        } break;
                        case OPMODE_SCALE: {
                            ScaleEntity(m_tmanager, entity, m_axis, viewProj, delta);
                        } break;
                    }
                }
            }

            if (input_MouseButtonPressed(input_MouseButton::LEFT) && !ImGuizmo::IsOver()) {
                Complete();
            }
            if (input_MouseButtonPressed(input_MouseButton::RIGHT)) {
                Cancel();
            }
        }
    }
} // namespace pge