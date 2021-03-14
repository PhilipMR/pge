#include "../include/edit_transform.h"
#include <imgui/imgui.h>

namespace pge
{
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

} // namespace pge