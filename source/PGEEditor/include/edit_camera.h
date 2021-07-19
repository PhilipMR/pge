#ifndef PGE_EDITOR_EDIT_CAMERA_H
#define PGE_EDITOR_EDIT_CAMERA_H

#include "edit_component.h"
#include "edit_command.h"
#include <game_camera.h>
#include <game_world.h>
#include <gfx_render_target.h>

namespace pge
{
    class edit_CameraEditor : public edit_ComponentEditor {
        game_World*          m_world;
        gfx_GraphicsAdapter* m_graphicsAdapter;
        gfx_RenderTarget     m_camPreviewRT;
        game_RenderPass      m_renderPass = game_RenderPass::LIGHTING;

    public:
        edit_CameraEditor(game_World* world, gfx_GraphicsAdapter* graphicsAdapter);
        virtual void UpdateAndDraw(const game_Entity& entity) override;
    };

    class edit_CommandCreateCamera : public edit_Command {
        game_World*           m_world;
        game_Entity           m_createdEntity;
        game_SerializedEntity m_sentity;

    public:
        edit_CommandCreateCamera(game_World* world);

        virtual void                         Do() override;
        virtual void                         Undo() override;
        static std::unique_ptr<edit_Command> Create(game_World* world);
    };
} // namespace pge

#endif