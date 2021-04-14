#include "../include/game_script.h"
#include <lua/lua.hpp>
#include <diag_assert.h>
#include <input_keyboard.h>

namespace pge
{
    static int
    TestFuncConstant(lua_State* state)
    {
        int n = lua_gettop(state);
        diag_Assert(n == 1);
        diag_Assert(lua_isnumber(state, 1));
        lua_Number num = lua_tonumber(state, 1);
        diag_LogDebugf("testPrintNum: %f", num);
        return 0;
    }

    static int
    TestIsQButtonDown(lua_State* state)
    {
        int n = lua_gettop(state);
        diag_Assert(n == 0);
        lua_pushboolean(state, input_KeyboardDown(input_KeyboardKey::Q));
        return 1;
    }

    // ==================================================
    // game_ScriptManager
    // ==================================================
    struct game_ScriptManager::ScriptAPIImpl {
        lua_State* luaState;
    };

    game_ScriptManager::game_ScriptManager(size_t capacity)
        : m_apiImpl(new ScriptAPIImpl)
        , m_scripts(new ScriptComponent[capacity])
        , m_numScripts(0)
    {
        m_apiImpl->luaState = luaL_newstate();
        diag_Assert(m_apiImpl->luaState != nullptr);
        lua_register(m_apiImpl->luaState, "testPrintNum", &TestFuncConstant);
        lua_register(m_apiImpl->luaState, "testIsQButtonDown", &TestIsQButtonDown);
    }

    game_ScriptManager::~game_ScriptManager()
    {
        lua_close(m_apiImpl->luaState);
    }

    void
    game_ScriptManager::GarbageCollect(const game_EntityManager& entityManager)
    {
        for (size_t aliveStreak = 0; m_numScripts > 0 && aliveStreak < 4;) {
            unsigned randIdx = rand() % m_numScripts;
            if (!entityManager.IsEntityAlive(m_scripts[randIdx].entity)) {
                DestroyScript(randIdx);
                aliveStreak = 0;
            } else {
                aliveStreak++;
            }
        }
    }

    void
    game_ScriptManager::CreateScript(const game_Entity& entity, const char* path)
    {
        diag_Assert(!HasScript(entity));
        game_ScriptId lid = m_numScripts++;
        m_entityMap.insert(std::make_pair(entity, lid));
        m_scripts[lid].entity = entity;
        m_scripts[lid].path   = path;
    }

    bool
    game_ScriptManager::HasScript(const game_Entity& entity) const
    {
        return m_entityMap.find(entity) != m_entityMap.end();
    }

    game_ScriptId
    game_ScriptManager::GetScriptId(const game_Entity& entity) const
    {
        diag_Assert(HasScript(entity));
        return m_entityMap.at(entity);
    }

    const char*
    game_ScriptManager::GetScriptPath(const game_ScriptId& id) const
    {
        diag_Assert(id < m_numScripts);
        return m_scripts[id].path.c_str();
    }

    void
    game_ScriptManager::SetScript(const game_ScriptId& id, const char* path)
    {
        diag_Assert(id < m_numScripts);
        m_scripts[id].path = path;
    }

    void
    game_ScriptManager::DestroyScript(const game_ScriptId& id)
    {
        diag_Assert(id < m_numScripts);
        game_Entity   entity = m_scripts[id].entity;
        game_ScriptId lastId = m_numScripts - 1;
        m_entityMap.erase(m_entityMap.find(entity));
        if (id != lastId) {
            m_scripts[id]           = m_scripts[lastId];
            game_Entity lastEntity  = m_scripts[lastId].entity;
            m_entityMap[lastEntity] = id;
        }
        m_numScripts--;
    }

    void
    game_ScriptManager::UpdateScripts()
    {
        for (size_t i = 0; i < m_numScripts; ++i) {
            const ScriptComponent& script = m_scripts[i];

            lua_State* L = m_apiImpl->luaState;
            luaL_dofile(L, script.path.c_str());
            lua_getglobal(L, "onUpdate");
            double dt = 1.0 / 60.0;
            lua_pushnumber(L, dt);
            lua_call(L, 1, 0);
        }
    }
} // namespace pge