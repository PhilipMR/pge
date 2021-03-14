#ifndef PGE_EDITOR_EDIT_TRANSFORM_H
#define PGE_EDITOR_EDIT_TRANSFORM_H

#include "edit_command.h"
#include "edit_component.h"
#include <game_transform.h>

namespace pge
{
    enum class edit_Axis
    {
        // clang-format off
        NONE,
        X, Y, Z,
        XY, XZ,YZ
        // clang-format on
    };

    class edit_CommandTranslate : public edit_Command {
        game_Entity            m_entity;
        math_Vec3              m_translation;
        game_TransformManager* m_tmanager;

    public:
        edit_CommandTranslate(const game_Entity& entity, const math_Vec3& translation, game_TransformManager* tm);
        virtual void Do() override;
        virtual void Undo() override;

        static std::unique_ptr<edit_Command> Create(const game_Entity& entity, const math_Vec3& translation, game_TransformManager* tm);
    };

    class edit_TransformEditor : public edit_ComponentEditor {
        game_TransformManager* m_tmanager;

    public:
        edit_TransformEditor(game_TransformManager* tm);
        virtual void UpdateAndDraw(const game_Entity& entity) override;
    };

    //    class game_TransformManager;
    //    class edit_TranslateTool {
    //        game_Entity            m_entity;
    //        edit_Axis              m_axis;
    //        game_TransformManager* m_tmanager;
    //        math_Vec3              m_initialPosition;
    //
    //    public:
    //        edit_TranslateTool(const game_Entity& entity, const edit_Axis& axis, game_TransformManager* m_tmanager);
    //        void Update(const math_Vec2& delta);
    //    };
} // namespace pge

#endif