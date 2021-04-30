#include "../include/edit_script.h"
#include <core_assert.h>
#include <imgui/imgui.h>
#include <imgui/TextEditor.h>
#include <ctime>
#include <fstream>

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
        for (const auto& m_scriptItem : m_scriptItems) {
            scriptPaths.push_back(m_scriptItem.path.c_str());
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
        core_AssertWithReason(curIdx >= 0, "The script that was assigned to the entity has been relocated or renamed.");
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


        static ImGuiJako::TextEditor textEditor;
        textEditor.SetLanguageDefinition(ImGuiJako::TextEditor::LanguageDefinition::Lua());
        bool needScriptLoad = isNewScript || nextIdx != curIdx;
        if (needScriptLoad) {
            std::string fileContents = core_ReadFile(scriptPaths[nextIdx]);
            textEditor.SetText(fileContents);
        }
        if (ImGui::Button("Save")) {
            std::ofstream file(scriptPaths[nextIdx]);
            std::string   text = textEditor.GetText();
            file.write(text.c_str(), text.size());
        }

        ImGui::SameLine();
        if (ImGui::Button("Remove script")) {
            m_scriptManager->DestroyScript(sid);
        }

        textEditor.Render("Script text");
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
        m_scriptItems = core_FSItemsWithExtension("data", "lua", true);
    }
} // namespace pge