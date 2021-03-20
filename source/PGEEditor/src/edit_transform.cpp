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
        auto tid   = m_tmanager->GetTransformId(m_entity);
        auto local = m_tmanager->GetLocal(tid);
        for (size_t i = 0; i < 3; ++i) {
            local[i][3] += m_translation[i];
        }
        m_tmanager->SetLocal(tid, local);
    }

    void
    edit_CommandTranslate::Undo()
    {
        auto tid   = m_tmanager->GetTransformId(m_entity);
        auto local = m_tmanager->GetLocal(tid);
        for (size_t i = 0; i < 3; ++i) {
            local[i][3] -= m_translation[i];
        }
        m_tmanager->SetLocal(tid, local);
    }

    std::unique_ptr<edit_Command>
    edit_CommandTranslate::Create(const game_Entity& entity, const math_Vec3& translation, game_TransformManager* tm)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandTranslate(entity, translation, tm));
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
        auto      tid   = m_tmanager->GetTransformId(entity);
        auto      world = m_tmanager->GetWorld(tid);
        math_Vec3 pos(world[0][3], world[1][3], world[2][3]);
        if (ImGui::DragFloat3("Position", &pos[0])) {
            world[0][3] = pos[0];
            world[1][3] = pos[1];
            world[2][3] = pos[2];
            m_tmanager->SetLocal(tid, world);
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
            default: {
                diag_AssertWithReason(false, "Unhandled axis!");
            } break;
        }
    }

    static void
    DrawAxis(const game_TransformManager* tm, const game_Entity& entity, const edit_Axis& axis)
    {
        auto        tid        = tm->GetTransformId(entity);
        auto        world      = tm->GetWorld(tid);
        auto        pos        = math_Vec3(world[0][3], world[1][3], world[2][3]);
        const float lineLength = 10000.0f;

        math_Vec3 axisVecs[2];
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

        math_Vec3 axisVecs[2];
        size_t    numAxisVecs = 0;
        GetAxisVectors(axis, axisVecs, &numAxisVecs);
        for (size_t i = 0; i < numAxisVecs; ++i) {
            math_Vec4 taxis    = viewProj * math_Vec4(axisVecs[i], 0);
            auto      taxlen   = math_Length(taxis);
            //diag_LogDebugf("taxlen = %f", taxlen);
            math_Vec2 axisDir  = math_Normalize(math_Vec2(taxis.x, taxis.y));
            float     stepSize = deltaMag * math_Dot(axisDir, deltaDir) * taxlen * 0.03f;
            pos += stepSize * axisVecs[i];
        }
        world[0][3] = pos.x;
        world[1][3] = pos.y;
        world[2][3] = pos.z;
        tm->SetLocal(tid, world);
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
        auto local        = m_tmanager->GetLocal(tid);
        m_initialPosition = math_Vec3(local[0][3], local[1][3], local[2][3]);
        m_entity          = entity;
        m_axis            = edit_Axis::NONE;
        m_hasBegun        = true;
    }


    void
    edit_TranslateTool::CompleteTranslation(edit_CommandStack* cstack)
    {
        diag_Assert(m_hasBegun);
        m_hasBegun = false;

        auto                  tid   = m_tmanager->GetTransformId(m_entity);
        auto                  world = m_tmanager->GetWorld(tid);
        math_Vec3             curPos(world[0][3], world[1][3], world[2][3]);
        math_Vec3             trans = curPos - m_initialPosition;
        cstack->Add(edit_CommandTranslate::Create(m_entity, trans, m_tmanager));
    }

    void
    edit_TranslateTool::CancelTranslation()
    {
        diag_Assert(m_hasBegun);
        m_hasBegun = false;

        auto tid    = m_tmanager->GetTransformId(m_entity);
        auto world  = m_tmanager->GetWorld(tid);
        world[0][3] = m_initialPosition.x;
        world[1][3] = m_initialPosition.y;
        world[2][3] = m_initialPosition.z;
        m_tmanager->SetLocal(tid, world);
    }

    void
    edit_TranslateTool::UpdateAndDraw(const math_Mat4x4& viewProj, const math_Vec2& delta)
    {
        if (m_hasBegun) {
            if (input_KeyboardDown(input_KeyboardKey::SHIFT)) {
                if (input_KeyboardPressed(input_KeyboardKey::X)) {
                    m_axis = edit_Axis::YZ;
                } else if (input_KeyboardPressed(input_KeyboardKey::Y)) {
                    m_axis = edit_Axis::XZ;
                } else if (input_KeyboardPressed(input_KeyboardKey::Z)) {
                    m_axis = edit_Axis::XY;
                }
            } else {
                if (input_KeyboardPressed(input_KeyboardKey::X)) {
                    m_axis = edit_Axis::X;
                } else if (input_KeyboardPressed(input_KeyboardKey::Y)) {
                    m_axis = edit_Axis::Y;
                } else if (input_KeyboardPressed(input_KeyboardKey::Z)) {
                    m_axis = edit_Axis::Z;
                }
            }


            DrawAxis(m_tmanager, m_entity, m_axis);
            TranslateEntity(m_tmanager, m_entity, m_axis, viewProj, delta);
        }
    }

} // namespace pge