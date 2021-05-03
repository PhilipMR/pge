#ifndef PGE_EDITOR_EDIT_SCRIPT_H
#define PGE_EDITOR_EDIT_SCRIPT_H

#include "edit_component.h"
#include <game_script.h>
#include <core_file_utils.h>

namespace pge
{
    class edit_ScriptEditor : public edit_ComponentEditor {
        game_ScriptManager*      m_scriptManager;
        time_t                   m_lastFileSync;
        std::vector<core_FSItem> m_scriptItems;

    public:
        edit_ScriptEditor(game_ScriptManager* sm);
        virtual void UpdateAndDraw(const game_Entity& entity) override;

    private:
        void SyncResourcePaths();
    };
} // namespace pge

#endif