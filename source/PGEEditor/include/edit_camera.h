#ifndef PGE_EDITOR_EDIT_CAMERA_H
#define PGE_EDITOR_EDIT_CAMERA_H

#include "edit_component.h"
#include <game_camera.h>
#include <game_world.h>
#include <gfx_render_target.h>

namespace pge
{
    class edit_CameraEditor : public edit_ComponentEditor {
        game_CameraManager*  m_cameraManager;
        gfx_GraphicsAdapter* m_graphicsAdapter;
        gfx_RenderTarget     m_camPreviewRT;
        game_World*          m_world;

    public:
        edit_CameraEditor(game_CameraManager* cm, gfx_GraphicsAdapter* graphicsAdapter, game_World* world);
        virtual void UpdateAndDraw(const game_Entity& entity) override;
    };
} // namespace pge

#endif