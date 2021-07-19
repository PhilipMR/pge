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

        static std::unique_ptr<edit_Command>
        Create(const game_Entity& entity, const math_Quat& initialRot, const math_Quat& newRot, game_TransformManager* tm);
    };

    class edit_CommandSetParent : public edit_Command {
        game_TransformId       m_transform;
        game_TransformId       m_originalParent;
        game_TransformId       m_newParent;
        game_TransformManager* m_tmanager;

    public:
        edit_CommandSetParent(const game_TransformId& transform, const game_TransformId& parent, game_TransformManager* tm);
        virtual void Do() override;
        virtual void Undo() override;

        static std::unique_ptr<edit_Command>
        Create(const game_TransformId& transform, const game_TransformId& parent, game_TransformManager* tm);
    };


    class edit_TransformEditor : public edit_ComponentEditor {
        game_TransformManager* m_tmanager;

    public:
        edit_TransformEditor(game_TransformManager* tm);
        virtual void UpdateAndDraw(const game_Entity& entity) override;
    };

    class edit_TransformGizmo {
        enum OpMode
        {
            OPMODE_NONE,
            OPMODE_TRANSLATE,
            OPMODE_ROTATE,
            OPMODE_SCALE
        } m_opMode;

        enum RelMode
        {
            RELMODE_WORLD,
            RELMODE_LOCAL
        } m_relMode;

        game_TransformManager* m_tmanager;
        edit_CommandStack*     m_cstack;
        edit_Axis              m_axis;
        bool                   m_isManipulating;

        struct {
            math_Vec3 position;
            math_Vec3 scale;
            math_Quat rotation;
        } m_initial;
        game_Entity m_entity;

        bool HasBegun() const;
        void Begin(const OpMode& opMode, const RelMode& relMode, const game_Entity& entity);
        void Cancel();
        void Complete();

    public:
        edit_TransformGizmo(game_TransformManager* tm, edit_CommandStack* cstack);
        bool IsVisible() const;
        void TransformEntity(const game_Entity& entity, const math_Mat4x4& view, const math_Mat4x4& proj);
    };


} // namespace pge

#endif