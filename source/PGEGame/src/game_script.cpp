#include "../include/game_script.h"
#include <lua/lua.hpp>
#include <core_assert.h>
#include <input_keyboard.h>

namespace pge
{
    static int
    ScriptAPI_is_action_pressed(lua_State* L)
    {
        return 0;
    }

    static int
    ScriptAPI_is_action_released(lua_State* L)
    {
        return 0;
    }

    static int
    ScriptAPI_is_action(lua_State* L)
    {
        return 0;
    }

    static int
    ScriptAPI_get_axis(lua_State* L)
    {
        return 0;
    }





    static int
    TestFuncConstant(lua_State* state)
    {
        int n = lua_gettop(state);
        core_Assert(n == 1);
        core_Assert(lua_isnumber(state, 1));
        lua_Number num = lua_tonumber(state, 1);
        core_LogDebugf("testPrintNum: %f", num);
        return 0;
    }

    static int
    TestIsQButtonDown(lua_State* state)
    {
        int n = lua_gettop(state);
        core_Assert(n == 0);
        lua_pushboolean(state, input_KeyboardDown(input_KeyboardKey::Q));
        return 1;
    }

    static int
    Print(lua_State* state)
    {
        int n = lua_gettop(state); 
        core_Assert(n == 1);
        core_Assert(lua_isstring(state, 1));
        const char* message = lua_tostring(state, 1);
        core_LogDebug(message);
        return 0;
    }

    static const luaL_Reg SCRIPT_API_FUNCTIONS[]
        = {{"testPrintNum", TestFuncConstant}, {"testIsQButtonDown", TestIsQButtonDown}, {"print", Print}, {"bla.print", Print}, {nullptr, nullptr}};

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
        core_Assert(m_apiImpl->luaState != nullptr);
        luaL_openlibs(m_apiImpl->luaState);
        lua_getglobal(m_apiImpl->luaState, "_G");
        luaL_setfuncs(m_apiImpl->luaState, SCRIPT_API_FUNCTIONS, 0);
        lua_pop(m_apiImpl->luaState, 1);
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
        core_Assert(!HasScript(entity));
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
        core_Assert(HasScript(entity));
        return m_entityMap.at(entity);
    }

    const char*
    game_ScriptManager::GetScriptPath(const game_ScriptId& id) const
    {
        core_Assert(id < m_numScripts);
        return m_scripts[id].path.c_str();
    }

    void
    game_ScriptManager::SetScript(const game_ScriptId& id, const char* path)
    {
        core_Assert(id < m_numScripts);
        m_scripts[id].path = path;
    }

    void
    game_ScriptManager::DestroyScript(const game_ScriptId& id)
    {
        core_Assert(id < m_numScripts);
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
    LuaErrorReport(lua_State* state)
    {
        core_LogErrorf("LUA Error: %s", lua_tostring(state, 1));
        lua_pop(state, 1);
    }

    void
    game_ScriptManager::UpdateScripts()
    {
        for (size_t i = 0; i < m_numScripts; ++i) {
            const ScriptComponent& script = m_scripts[i];

            lua_State* L = luaL_newstate(); // m_apiImpl->luaState;
            luaL_openlibs(L);
            lua_getglobal(L, "_G");
            luaL_setfuncs(L, SCRIPT_API_FUNCTIONS, 0);
            lua_pop(L, 1);

            luaL_dofile(L, script.path.c_str());
            lua_getglobal(L, "onUpdate");
            double dt = 1.0 / 60.0;
            lua_pushnumber(L, dt);
            int error = lua_pcall(L, 1, 0, 0);
            if (error != LUA_OK) {
                LuaErrorReport(L);
            }
            lua_close(L);
        }
    }
} // namespace pge