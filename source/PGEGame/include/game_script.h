#ifndef PGE_GAME_GAME_SCRIPT_H
#define PGE_GAME_GAME_SCRIPT_H

#include "game_entity.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace pge
{
    using game_ScriptId                           = unsigned long long;
    constexpr game_ScriptId game_ScriptId_Invalid = -1;
    class game_ScriptManager {
        struct ScriptAPIImpl;
        std::unique_ptr<ScriptAPIImpl> m_apiImpl;

        struct ScriptComponent {
            game_Entity entity;
            std::string path;
            // TODO: Parameters (and maybe the script itself should be a static resource)
        };
        std::unordered_map<game_Entity, game_ScriptId> m_entityMap;
        std::unique_ptr<ScriptComponent[]>             m_scripts;
        size_t                                         m_numScripts;

    public:
        game_ScriptManager(size_t capacity);
        ~game_ScriptManager();
        void GarbageCollect(const game_EntityManager& entityManager);

        void          CreateScript(const game_Entity& entity, const char* path);
        bool          HasScript(const game_Entity& entity) const;
        game_ScriptId GetScriptId(const game_Entity& entity) const;
        const char*   GetScriptPath(const game_ScriptId& id) const;
        void          SetScript(const game_ScriptId& id, const char* path);
        void          DestroyScript(const game_ScriptId& id);

        void UpdateScripts();
    };
} // namespace pge

#endif