#ifndef PGE_EDITOR_EDIT_EDITOR_VIEWS_H
#define PGE_EDITOR_EDIT_EDITOR_VIEWS_H

#include "edit_command.h"
#include "edit_transform.h"
#include "edit_camera.h"
#include "edit_light.h"

#include <core_log.h>
#include <game_world.h>

namespace pge
{
    static const char* edit_PATH_TO_LAYOUT_INI = "layout.ini";

    /**
     * Draws the main menu bar (i.e. file, edit, etc.)
     * @param world The world that can be manipulated.
     * @param cstack The command stack to allow undos/redos.
     */
    void edit_DrawMainMenuBar(game_World* world, edit_CommandStack* cstack);

    /**
     * A class representing the log view in the editor.
     * It can draw within an existing ImGui context.
     * Draws filter-checkboxes and the log text.
     */
    class edit_LogView {
        static const std::uint8_t LOG_FLAG_DEBUG   = 1 << core_LogRecord::RecordType::DEBUG;
        static const std::uint8_t LOG_FLAG_WARNING = 1 << core_LogRecord::RecordType::WARNING;
#undef ERROR
        static const std::uint8_t LOG_FLAG_ERROR = 1 << core_LogRecord::RecordType::ERROR;

        std::uint8_t m_logMask = LOG_FLAG_DEBUG | LOG_FLAG_WARNING | LOG_FLAG_ERROR;

    public:
        void DrawOnGUI();
    };

    /**
     * A class representing the gizmo view in the editor.
     * It can draw outside of an ImGui context, to show bounding boxes,
     * billboard sprites and manipulatable transformation gizmos.
     */
    class edit_GizmoView {
        edit_TransformGizmo  m_transformGizmo;
        game_World*          m_world;
        const gfx_Texture2D* m_pointLightIcon;
        const gfx_Texture2D* m_cameraIcon;

    public:
        edit_GizmoView(game_World* world, res_ResourceManager* resources, edit_CommandStack* cstack);
        bool IsActive() const;
        void UpdateAndDraw(bool               drawGrid,
                           bool               drawGizmos,
                           const game_Entity& selectedEntity,
                           const math_Vec2&   windowPos,
                           const math_Vec2&   windowSize,
                           const math_Mat4x4& view,
                           const math_Mat4x4& proj);
    };


    /**
     * A class representing the editor view (main editing panel) in the editor.
     * It can draw within an existing ImGui context.
     */
    class edit_EditView {
        class EditCamera {
            game_TransformManager m_transformManager;
            game_CameraManager    m_cameraManager;
            const game_Entity     CAM_ENTITY = 0;

        public:
            EditCamera()
                : m_transformManager(1)
                , m_cameraManager(&m_transformManager)
            {
                m_cameraManager.CreateCamera(CAM_ENTITY);
                m_cameraManager.SetLookAt(CAM_ENTITY, math_Vec3(0, -10, 0), math_Vec3(0, 0, 0));
            }

            // clang-format off
            inline math_Mat4x4 GetView() const { return m_cameraManager.GetViewMatrix(CAM_ENTITY); }
            inline const math_Mat4x4& GetProjection() const { return m_cameraManager.GetProjectionMatrix(CAM_ENTITY); }
            inline void UpdateFPS() { m_cameraManager.UpdateFPS(CAM_ENTITY); }
            inline const float GetAspect() { return m_cameraManager.GetPerspective(CAM_ENTITY).aspect; }
            // clang-format on
        } m_editCamera;

        gfx_GraphicsAdapter* m_graphicsAdapter;
        game_World*          m_world;
        edit_CommandStack*   m_cstack;
        edit_GizmoView       m_gizmoView;
        gfx_RenderTarget     m_rtGameMs;
        gfx_RenderTarget     m_rtGame;
        const res_Effect*    m_multisampleEffect;

        math_Vec2 m_viewPos;
        math_Vec2 m_viewSize;
        bool      m_isHovered;

        const float CLICK_RADIUS = 15.0f;
        math_Vec2   m_cursorDown;
        bool        m_showEntityContext = false;

        bool m_drawGrid   = true;
        bool m_drawGizmos = true;

        game_Entity EntityAtCursor() const;

    public:
        edit_EditView(gfx_GraphicsAdapter* graphicsAdapter,
                      res_ResourceManager* resources,
                      game_World*          world,
                      edit_CommandStack*   cstack,
                      unsigned             width,
                      unsigned             height);

        void DrawOnGUI(game_Entity* selectedEntity);
        const bool       IsHovered() const;
    };


    /**
     * A class representing the entity transform hierarchy view in the editor.
     * It can draw within an existing ImGui context.
     * Each entity with a transform will appear the entity hierarchy and is selectable,
     * can be dragged-and-dropped onto another node, and has a context popup menu.
     */
    class edit_EntityHierarchyView {
        game_Entity m_hoveredEntity;

        bool DrawEntityNode(game_World*               world,
                            const game_Entity&        entity,
                            game_Entity*              selectedEntity,
                            std::vector<game_Entity>* entitiesRemove,
                            bool                      isLeaf);
        void DrawLocalTree(game_World* world, const game_Entity& entity, game_Entity* selectedEntity, std::vector<game_Entity>* entitiesRemove);

    public:
        void DrawOnGUI(game_World* world, game_Entity* selectedEntity, edit_CommandStack* cstack);
    };


    /**
     * A class representing the resource view in the editor.
     * It can draw within an existing ImGui context.
     * The resource view consists of two sub-views; the file explorer and the resource preview.
     * The client can navigate the file explorer and select a file, which can then be previewed.
     */
    class edit_ResourceView {
        gfx_GraphicsAdapter* m_graphicsAdapter;
        game_World*          m_world;
        res_ResourceManager* m_resources;

        const char* ROOT_DIR     = "data\\";
        std::string m_currentDir = ROOT_DIR;
        std::string m_selectedFile;

        static const std::uint8_t FILTER_FLAG_MESH = 1 << 0;
        static const std::uint8_t FILTER_FLAG_MAT  = 1 << 1;
        std::uint8_t              m_filterMask     = FILTER_FLAG_MESH | FILTER_FLAG_MAT;

        constexpr static const math_Vec2 PREVIEW_RESOLUTION = math_Vec2(600.f, 600.f);
        gfx_RenderTarget                 m_previewRT;
        game_Renderer                    m_previewRenderer;

        void* RenderMeshPreviewTexture(const res_Mesh* mesh, const res_Material* material);

    public:
        edit_ResourceView(gfx_GraphicsAdapter* graphicsAdapter,
                          gfx_GraphicsDevice*  graphicsDevice,
                          game_World*          world,
                          res_ResourceManager* resources);

        void DrawOnGUI(const game_Entity& selectedEntity, edit_CommandStack* cstack);
        void DrawPreviewOnGUI();
    };


    /**
     * A class representing the inspector view in the editor.
     * It can draw within an existing ImGui context.
     * The inspector displays component editing tools.
     */
    class edit_InspectorView {
        game_World*                                        m_world;
        edit_TransformEditor                               m_transformEditor;
        edit_LightEditor                                   m_lightEditor;
        edit_CameraEditor                                  m_cameraEditor;
        std::vector<std::unique_ptr<edit_ComponentEditor>> m_componentEditors;

    public:
        edit_InspectorView(game_World* world, gfx_GraphicsAdapter* graphicsAdapter, res_ResourceManager* resources);
        void DrawOnGUI(const game_Entity& selectedEntity);
    };
} // namespace pge

#endif