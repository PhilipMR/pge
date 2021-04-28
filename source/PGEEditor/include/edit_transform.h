#ifndef PGE_EDITOR_EDIT_TRANSFORM_H
#define PGE_EDITOR_EDIT_TRANSFORM_H

#include "edit_command.h"
#include "edit_component.h"
#include <game_transform.h>
#include <math_vec2.h>

namespace pge
{
    enum class edit_Axis
    {
        NONE,
        X,
        Y,
        Z,
        XY,
        XZ,
        YZ,
        XYZ
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

    class edit_CommandScale : public edit_Command {
        game_Entity            m_entity;
        math_Vec3              m_scale;
        game_TransformManager* m_tmanager;

    public:
        edit_CommandScale(const game_Entity& entity, const math_Vec3& scale, game_TransformManager* tm);
        virtual void Do() override;
        virtual void Undo() override;

        static std::unique_ptr<edit_Command> Create(const game_Entity& entity, const math_Vec3& scale, game_TransformManager* tm);
    };

    class edit_CommandSetRotation : public edit_Command {
        game_Entity            m_entity;
        math_Quat              m_initialRotation;
        math_Quat              m_rotation;
        game_TransformManager* m_tmanager;

    public:
        edit_CommandSetRotation(const game_Entity& entity, const math_Quat& initialRot, const math_Quat& newRot, game_TransformManager* tm);
        virtual void Do() override;
        virtual void Undo() override;

        static std::unique_ptr<edit_Command> Create(const game_Entity& entity, const math_Quat& initialRot, const math_Quat& newRot, game_TransformManager* tm);
    };


    class edit_TransformEditor : public edit_ComponentEditor {
        game_TransformManager* m_tmanager;

    public:
        edit_TransformEditor(game_TransformManager* tm);
        virtual void UpdateAndDraw(const game_Entity& entity) override;
    };

    class edit_TranslateTool {
        game_TransformManager* m_tmanager;
        game_Entity            m_entity;
        edit_Axis              m_axis;
        math_Vec3              m_initialPosition;
        bool                   m_hasBegun;

    public:
        edit_TranslateTool(game_TransformManager* tm);
        void BeginTranslation(const game_Entity& entity);
        void CompleteTranslation(edit_CommandStack* cstack);
        void CancelTranslation();
        void UpdateAndDraw(const math_Mat4x4& view, const math_Mat4x4& proj, const math_Vec2& delta);
    };

    class edit_ScalingTool {
        game_TransformManager* m_tmanager;
        game_Entity            m_entity;
        edit_Axis              m_axis;
        math_Vec3              m_initialScale;
        bool                   m_hasBegun;

    public:
        edit_ScalingTool(game_TransformManager* tm);
        void BeginScale(const game_Entity& entity);
        void CompleteScale(edit_CommandStack* cstack);
        void CancelScale();
        void UpdateAndDraw(const math_Mat4x4& view, const math_Mat4x4& proj, const math_Vec2& delta);
    };

    class edit_RotationTool {
        game_TransformManager* m_tmanager;
        game_Entity            m_entity;
        edit_Axis              m_axis;
        math_Quat              m_initialRot;
        bool                   m_hasBegun;

    public:
        edit_RotationTool(game_TransformManager* tm);
        void BeginRotation(const game_Entity& entity);
        void CompleteRotation(edit_CommandStack* cstack);
        void CancelRotation();
        void UpdateAndDraw(const math_Mat4x4& view, const math_Mat4x4& proj, const math_Vec2& delta);
    };

} // namespace pge

#endif