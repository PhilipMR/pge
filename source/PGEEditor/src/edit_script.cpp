#include "../include/edit_script.h"
#include <ctime>
#include <imgui/imgui.h>

namespace pge
{
    edit_ScriptEditor::edit_ScriptEditor(game_ScriptManager* sm)
        : m_scriptManager(sm)
        , m_lastFileSync()
    {}

    void
    edit_ScriptEditor::UpdateAndDraw(const game_Entity& entity)
    {
        if (!ImGui::CollapsingHeader("Script"))
            return;

        bool isNewScript = false;
        if (!m_scriptManager->HasScript(entity)) {
            if (ImGui::Button("Add script")) {
                m_scriptManager->CreateScript(entity, "");
                isNewScript = true;
            } else {
                return;
            }
        }
        SyncResourcePaths();

        std::vector<const char*> scriptPaths;
        scriptPaths.reserve(m_scriptItems.size());
        for (size_t i = 0; i < m_scriptItems.size(); ++i) {
            scriptPaths.push_back(m_scriptItems[i].path.c_str());
        }

        // Get active script index
        game_ScriptId sid           = m_scriptManager->GetScriptId(entity);
        std::string   curScriptPath = m_scriptManager->GetScriptPath(sid);
        int           curIdx        = 0;
        if (!curScriptPath.empty()) {
            curIdx = -1;
            for (int i = 0; i < scriptPaths.size(); ++i) {
                if (curScriptPath == scriptPaths[i]) {
                    curIdx = i;
                    break;
                }
            }
        }
        diag_AssertWithReason(curIdx >= 0, "The script that was assigned to the entity has been relocated or renamed.");
        if (curScriptPath.empty()) {
            m_scriptManager->SetScript(sid, scriptPaths[curIdx]);
        }

        // Mesh dropdown select
        int nextIdx = curIdx;
        if (ImGui::Combo("Script##combo", &nextIdx, &scriptPaths[0], scriptPaths.size())) {
            if (curIdx != nextIdx) {
                m_scriptManager->SetScript(sid, scriptPaths[nextIdx]);
            }
        }


        static char scriptContents[4096];
        bool               needScriptLoad = isNewScript || nextIdx != curIdx;
        if (needScriptLoad) {
            std::string fileContents = os_ReadFile(scriptPaths[nextIdx]);
            memset(scriptContents, 0, sizeof(scriptContents));
            strcpy_s(scriptContents, fileContents.c_str());
        }
        ImGui::InputTextMultiline("##scriptcontent", scriptContents, sizeof(scriptContents));
        if (ImGui::Button("Save")) {
            std::ofstream file(scriptPaths[nextIdx]);
            file.write(scriptContents, strlen(scriptContents));
        }

        ImGui::SameLine();
        if (ImGui::Button("Remove script")) {
            m_scriptManager->DestroyScript(sid);
        }
    }

    void
    edit_ScriptEditor::SyncResourcePaths()
    {
        static const unsigned SECONDS_PER_SYNC = 10;
        time_t                tm;
        time(&tm);
        double diffSecs = difftime(tm, m_lastFileSync);
        if (diffSecs < SECONDS_PER_SYNC)
            return;
        else
            m_lastFileSync = tm;
        m_scriptItems = os_ListItemsWithExtension("data", "lua", true);
    }
} // namespace pge