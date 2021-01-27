#ifdef _DEBUG
#    define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#    define malloc(size) _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__)
#    define free(data)   _free_dbg(data, _NORMAL_BLOCK)
#    define _CRTDBG_MAP_ALLOC
#    include <stdlib.h>
#    include <crtdbg.h>
#endif

#include <os_display_win32.h>
#include <gfx_graphics_adapter_d3d11.h>
#include <gfx_graphics_device.h>
#include <gfx_buffer.h>
#include <math_vec4.h>
#include <math_mat4x4.h>
#include <math_quat.h>
#include <res_resource_manager.h>

static LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_DESTROY)
        PostQuitMessage(0);
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

namespace pge
{
    class Transform {
        math_Vec3 m_position;
        math_Vec3 m_scale;
        math_Quat m_rotation;

    public:
        Transform()
            : m_position()
            , m_scale(math_Vec3::One())
            , m_rotation()
        {}

        Transform(const math_Vec3& position, const math_Vec3& scale, const math_Quat& rotation)
            : m_position(position)
            , m_scale(scale)
            , m_rotation(rotation)
        {}

        void
        Translate(const math_Vec3& translation)
        {
            m_position += translation;
        }

        void
        Scale(const math_Vec3& scale)
        {
            m_scale = math_Vec3(m_scale.x * scale.x, m_scale.y * scale.y, m_scale.z * scale.z);
        }

        void
        Rotate(const math_Vec3& axis, float degrees)
        {
            m_rotation = math_Rotate(m_rotation, axis, degrees);
        }

        math_Vec3
        GetPosition() const
        {
            return m_position;
        }

        math_Vec3
        GetScale() const
        {
            return m_scale;
        }

        math_Quat
        GetRotation() const
        {
            return m_rotation;
        }

        void
        SetPosition(const math_Vec3& position)
        {
            m_position = position;
        }

        void
        SetScale(const math_Vec3& scale)
        {
            m_scale = scale;
        }

        void
        SetRotation(const math_Quat& rotation)
        {
            m_rotation = rotation;
        }

        math_Mat4x4
        GetModelMatrix() const
        {
            return math_CreateTranslationMatrix(m_position) * math_CreateRotationMatrix(m_rotation) * math_CreateScaleMatrix(m_scale);
        }

        void
        LookAt(const math_Vec3& target)
        {}
    };


    class Camera {
        Transform   m_transform;
        math_Mat4x4 m_projectionMatrix;

    public:
        Camera(float fov, float aspect, float nearClip, float farClip)
        {
            m_projectionMatrix = math_Perspective(fov, aspect, nearClip, farClip);
        }

        Transform*
        GetTransform()
        {
            return &m_transform;
        }

        math_Mat4x4
        GetViewMatrix() const
        {
            return math_LookAt(m_transform.GetPosition(), math_Vec3(), math_Vec3(0, 1, 0));
        }

        math_Mat4x4
        GetProjectionMatrix() const
        {
            return m_projectionMatrix;
        }
    };
} // namespace pge


int
main()
{
    using namespace pge;

#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
#endif

    // Scope main body so everything is deallocated by the end of main.
    {
        // Create graphics context
        os_DisplayWin32          display("PGE Sandbox", 1280, 720, WindowProc);
        gfx_GraphicsAdapterD3D11 graphicsAdapter(display.GetWindowHandle(), display.GetWidth(), display.GetHeight());

        // Load resources
        res_ResourceManager resources(&graphicsAdapter);
        const res_Mesh*     mesh     = resources.GetMesh(R"(C:\Users\phili\Desktop\suzanne\meshes\Suzanne.001.mesh)");
        const res_Material* material = resources.GetMaterial(R"(C:\Users\phili\Desktop\suzanne.mat)");

        Camera camera(60.0f, 1280.0f / 720.0f, 0.01f, 100.0f);
        camera.GetTransform()->SetPosition(math_Vec3(0, 0, 5));
        camera.GetTransform()->LookAt(math_Vec3(0, 0, 0));

        Transform transform;

        // Create transforms constant buffer
        struct CBTransforms {
            math_Mat4x4 modelMatrix;
            math_Mat4x4 viewMatrix;
            math_Mat4x4 projMatrix;
        } transforms;
        transforms.modelMatrix = transform.GetModelMatrix();
        transforms.viewMatrix  = camera.GetViewMatrix();
        transforms.projMatrix  = camera.GetProjectionMatrix();
        gfx_ConstantBuffer transformCBuffer(&graphicsAdapter, &transforms, sizeof(CBTransforms), gfx_BufferUsage::DYNAMIC);

        // Rotate animation lambda
        auto rotateClockwise = [&](float speed) {
            transform.Rotate(math_Vec3(0, 1, 0), -speed);
            transforms.modelMatrix = transform.GetModelMatrix();
            transformCBuffer.Update(&transforms, sizeof(CBTransforms));
        };

        // Bind the transform buffer, the mesh and the active material
        transformCBuffer.BindVS(0);
        mesh->Bind();
        material->Bind();

        // Create and use a graphics device for in the draw loop
        gfx_GraphicsDevice graphicsDevice(&graphicsAdapter);
        while (!display.IsCloseRequested()) {
            display.HandleEvents();
            graphicsDevice.Clear();
            rotateClockwise(360.0f / 60.0f * 0.25f);
            graphicsDevice.DrawIndexed(gfx_PrimitiveType::TRIANGLELIST, 0, mesh->GetNumTriangles() * 3);
            graphicsDevice.Present();
        }
    }

    return 0;
}