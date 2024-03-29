#include "../include/edit_entity.h"
#include <imgui/imgui.h>
#include <sstream>

namespace pge
{
    // ===============================
    // edit_EntityNameEditor
    // ===============================
    edit_EntityNameEditor::edit_EntityNameEditor(game_EntityManager* emanager)
        : m_emanager(emanager)
    {}

    void
    edit_EntityNameEditor::UpdateAndDraw(const game_Entity& entity)
    {
        if (!ImGui::CollapsingHeader("Name"))
            return;

        char        textBuffer[64];
        std::string name = m_emanager->GetName(entity);
        strcpy_s(textBuffer, name.c_str());

        std::stringstream ss;
        ss << "##editname" << entity.id;
        if (ImGui::InputText(ss.str().c_str(), textBuffer, sizeof(textBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
            m_emanager->SetName(entity, textBuffer);
    }


    // ===============================
    // edit_CommandDeleteEntity
    // ===============================
    edit_CommandDeleteEntity::edit_CommandDeleteEntity(const game_Entity& entity, game_World* world)
        : m_entity(entity)
        , m_world(world)
        , m_sentity(world->SerializeEntity(entity))
    {}

    void
    edit_CommandDeleteEntity::Do()
    {
        core_Assert(m_world->GetEntityManager()->IsEntityAlive(m_entity));
        m_world->GetEntityManager()->DestroyEntity(m_entity);
    }

    void
    edit_CommandDeleteEntity::Undo()
    {
        core_Assert(!m_world->GetEntityManager()->IsEntityAlive(m_entity));
        m_world->InsertSerializedEntity(m_sentity, m_entity);
    }

    std::unique_ptr<edit_Command>
    edit_CommandDeleteEntity::Create(const game_Entity& entity, game_World* world)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandDeleteEntity(entity, world));
    }


    // ===============================
    // edit_CommandCreateEntity
    // ===============================
    edit_CommandCreateEntity::edit_CommandCreateEntity(game_World* world)
        : m_world(world)
        , m_createdEntity(game_EntityId_Invalid)
    {}

    game_Entity
    edit_CommandCreateEntity::GetCreatedEntity() const
    {
        return m_createdEntity;
    }

    void
    edit_CommandCreateEntity::Do()
    {
        if (m_createdEntity == game_EntityId_Invalid) {
            m_createdEntity = m_world->GetEntityManager()->CreateEntity();
        } else {
            m_world->InsertSerializedEntity(m_sentity, m_createdEntity);
        }
    }

    void
    edit_CommandCreateEntity::Undo()
    {
        core_Assert(m_createdEntity != game_EntityId_Invalid);
        m_sentity = m_world->SerializeEntity(m_createdEntity);
        m_world->GetEntityManager()->DestroyEntity(m_createdEntity);
    }

    std::unique_ptr<edit_Command>
    edit_CommandCreateEntity::Create(game_World* world)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandCreateEntity(world));
    }


    // ===============================
    // edit_CommandDuplicateEntity
    // ===============================
    edit_CommandDuplicateEntity::edit_CommandDuplicateEntity(game_World* world, const game_Entity& entity)
        : m_world(world)
        , m_sentity(world->SerializeEntity(entity))
        , m_duplicate(game_EntityId_Invalid)
    {}

    game_Entity
    edit_CommandDuplicateEntity::GetCreatedEntity() const
    {
        return m_duplicate;
    }

    void
    edit_CommandDuplicateEntity::Do()
    {
        if (m_duplicate == game_EntityId_Invalid) {
            m_duplicate = m_world->InsertSerializedEntity(m_sentity);
            std::stringstream ss;
            ss << m_world->GetEntityManager()->GetName(m_duplicate) << "_copy";
            m_world->GetEntityManager()->SetName(m_duplicate, ss.str().c_str());
        } else {
            m_world->InsertSerializedEntity(m_sentity, m_duplicate);
        }
    }

    void
    edit_CommandDuplicateEntity::Undo()
    {
        core_Assert(m_duplicate != game_EntityId_Invalid);
        m_world->GetEntityManager()->DestroyEntity(m_duplicate);
    }

    std::unique_ptr<edit_Command>
    edit_CommandDuplicateEntity::Create(game_World* scene, const game_Entity& entity)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandDuplicateEntity(scene, entity));
    }
} // namespace pge