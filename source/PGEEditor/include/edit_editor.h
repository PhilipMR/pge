#ifndef PGE_EDITOR_EDIT_EDITOR_H
#define PGE_EDITOR_EDIT_EDITOR_H

#include <game_entity.h>
#include <math_vec2.h>
#include <math_vec3.h>
#include <res_resource_manager.h>
#include <vector>
#include <game_transform.h>

namespace pge
{
    class os_Display;
    class gfx_GraphicsAdapter;
    class gfx_RenderTarget;
    class game_Scene;

    void edit_Initialize(os_Display* display, gfx_GraphicsAdapter* graphics);
    void edit_Shutdown();
    void edit_BeginFrame();
    void edit_EndFrame();

    class game_Scene;

    enum class edit_EditMode
    {
        NONE,
        TRANSLATE,
    };

    enum class edit_Axis
    {
        NONE,
        X,
        Y,
        Z,
        XY,
        XZ,
        YZ
    };

    class edit_Command {
    public:
        virtual ~edit_Command() = default;
        virtual void Do()       = 0;
        virtual void Undo()     = 0;
    };

    class edit_CommandTranslate : public edit_Command {
        game_Entity            m_entity;
        math_Vec3              m_translation;
        game_TransformManager* m_tmanager;

    public:
        edit_CommandTranslate(const game_Entity& entity, const math_Vec3& translation, game_TransformManager* tm)
            : m_entity(entity)
            , m_translation(translation)
            , m_tmanager(tm)
        {}

        virtual void
        Do() override
        {
            auto tid   = m_tmanager->GetTransformId(m_entity);
            auto local = m_tmanager->GetLocal(tid);
            for (size_t i = 0; i < 3; ++i) {
                local[i][3] += m_translation[i];
            }
            m_tmanager->SetLocal(tid, local);
        }

        virtual void
        Undo() override
        {
            auto tid   = m_tmanager->GetTransformId(m_entity);
            auto local = m_tmanager->GetLocal(tid);
            for (size_t i = 0; i < 3; ++i) {
                local[i][3] -= m_translation[i];
            }
            m_tmanager->SetLocal(tid, local);
        }

        static std::unique_ptr<edit_Command>
        Create(const game_Entity& entity, const math_Vec3& translation, game_TransformManager* tm)
        {
            return std::unique_ptr<edit_Command>(new edit_CommandTranslate(entity, translation, tm));
        }
    };

    class edit_CommandStack {
        std::vector<std::unique_ptr<edit_Command>> m_stack;
        int                                        m_cursor;

    public:
        edit_CommandStack()
            : m_cursor(-1)
        {}

        void
        Add(std::unique_ptr<edit_Command> command)
        {
            if (!m_stack.empty()) {
                for (size_t i = m_stack.size() - 1; i > m_cursor; i--) {
                    m_stack.pop_back();
                }
            }
            m_stack.push_back(std::move(command));
            m_cursor = m_stack.size() - 1;
        }

        void
        Do(std::unique_ptr<edit_Command> command)
        {
            if (!m_stack.empty()) {
                for (size_t i = m_stack.size() - 1; i > m_cursor; i--) {
                    m_stack.pop_back();
                }
            }
            command->Do();
            m_stack.push_back(std::move(command));
            m_cursor = m_stack.size() - 1;
        }

        void
        Undo()
        {
            if (m_cursor >= 0) {
                m_stack[m_cursor--]->Undo();
            }
        }

        void
        Redo()
        {
            if (m_stack.empty())
                return;
            if (m_cursor < (int)m_stack.size() - 1) {
                m_stack[++m_cursor]->Do();
            }
        }
    };

    class edit_Editor {
        edit_EditMode m_editMode;
        edit_Axis     m_editAxis;
        math_Vec3     m_preTransformPosition;
        game_Entity   m_selectedEntity;
        math_Vec2     m_gameWindowPos;
        math_Vec2     m_gameWindowSize;

        edit_CommandStack m_commandStack;

    public:
        edit_Editor();
        void HandleEvents(game_Scene* scene);
        void DrawMenuBar(game_Scene* scene);
        bool DrawGameView(game_Scene* scene, const gfx_RenderTarget* target, res_ResourceManager* resources);
        void DrawEntityTree(game_Scene* scene);
        void DrawInspector(game_Scene* scene, res_ResourceManager* resources);
        void DrawExplorer();
    };
} // namespace pge

#endif