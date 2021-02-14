#include "../include/edit_editor.h"

#include <imgui/backends/imgui_impl_dx11.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <os_display_win32.h>
#include <gfx_graphics_adapter_d3d11.h>
#include <diag_assert.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace pge
{
    static bool s_isInitialized = false;

    void
    edit_Win32Events(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
    }

    void
    edit_Gui_Initialize(os_Display* display, gfx_GraphicsAdapter* graphics)
    {
        diag_Assert(!s_isInitialized);

        os_DisplayWin32* displayWin32 = reinterpret_cast<os_DisplayWin32*>(display);
        diag_Assert(displayWin32 != nullptr);

        gfx_GraphicsAdapterD3D11* graphicsD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphics);
        diag_Assert(graphicsD3D11 != nullptr);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        //        ImGui::StyleColorsDark();
        ImGui::StyleColorsLight();

        ImGui_ImplWin32_Init(displayWin32->GetWindowHandle());
        ImGui_ImplDX11_Init(graphicsD3D11->GetDevice(), graphicsD3D11->GetDeviceContext());
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // ImGui::GetIO().ConfigDockingWithShift = true;
        ImGui::GetIO().Fonts->AddFontFromFileTTF("data/fonts/Roboto-Regular.ttf", 16);

        s_isInitialized = true;
    }

    void
    edit_Gui_Shutdown()
    {
        diag_Assert(s_isInitialized);
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void
    edit_Gui_BeginFrame()
    {
        diag_Assert(s_isInitialized);
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void
    edit_Gui_EndFrame()
    {
        diag_Assert(s_isInitialized);
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    bool
    edit_Gui_IsFocused()
    {
        if (!s_isInitialized)
            return false;
        return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemHovered();
    }

    void
    edit_DrawLeftPanel()
    {
        ImGuiWindowFlags flags = 0;
        flags |= ImGuiWindowFlags_NoMove;
        flags |= ImGuiWindowFlags_NoNav;
        flags |= ImGuiWindowFlags_NoCollapse;

        static const float panelSizeMinWidth = 150.0f;
        static const float panelSizeMaxWidth = 350.0f;
        static const float menuBarHeight     = 0.0f;

        float displayHeight = ImGui::GetIO().DisplaySize.y;
        ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight));
        ImGui::SetNextWindowSizeConstraints(ImVec2(panelSizeMinWidth, displayHeight), ImVec2(panelSizeMaxWidth, displayHeight));
        if (ImGui::Begin("Scene panel", nullptr, flags)) {
            ImGui::End();
        }
    }

    void
    edit_DrawRightPanel()
    {
        // ImGui::DockSpace(0, ImVec2(100, 1080));

        ImGui::Begin("Inspection panel", nullptr, 0);
        ImGui::End();

        ImGui::Begin("dewded panel", nullptr, 0);
        ImGui::End();

        ImGui::Begin("dedede panel", nullptr, 0);
        ImGui::End();
    }

    void edit_DrawTopPanel();
    void edit_DrawBottomPanel();
} // namespace pge