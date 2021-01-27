#include <stdio.h>
#include <imgui/backends/imgui_impl_dx11.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/imgui.h>
#include <os_display_win32.h>
#include <gfx_graphics_adapter_d3d11.h>
#include <gfx_graphics_device.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;
    if (uMsg == WM_DESTROY)
        PostQuitMessage(0);
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static void
ImGui_Init(pge::os_DisplayWin32& display, pge::gfx_GraphicsAdapterD3D11& adapter)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(display.GetWindowHandle());
    ImGui_ImplDX11_Init(adapter.GetDevice(), adapter.GetDeviceContext());
    ImGui::GetIO().IniFilename = nullptr;
}

static void
ImGui_NewFrame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

static void
ImGui_EndFrame()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

static void
ImGui_Shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}


// ===== editor.h
class EditorMenu {
public:
    void
    View(int displayWidth)
    {
        ImGuiWindowFlags windowFlags = 0;
        windowFlags |= ImGuiWindowFlags_NoTitleBar;
        windowFlags |= ImGuiWindowFlags_MenuBar;
        windowFlags |= ImGuiWindowFlags_NoScrollbar;
        windowFlags |= ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoResize;
        windowFlags |= ImGuiWindowFlags_NoCollapse;
        windowFlags |= ImGuiWindowFlags_NoNav;
        windowFlags |= ImGuiWindowFlags_NoBackground;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(displayWidth, 0));
        if (!ImGui::Begin("EditorMenu", nullptr, windowFlags)) {
            ImGui::End();
            return;
        }
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Menu")) {
                ImGui::MenuItem("(demo menu)", NULL, false, false);
                if (ImGui::MenuItem("New")) {}
                if (ImGui::MenuItem("Open", "Ctrl+O")) {}
                if (ImGui::BeginMenu("Open Recent")) {
                    ImGui::MenuItem("fish_hat.c");
                    ImGui::MenuItem("fish_hat.inl");
                    ImGui::MenuItem("fish_hat.h");
                    if (ImGui::BeginMenu("More..")) {
                        ImGui::MenuItem("Hello");
                        ImGui::MenuItem("Sailor");
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("Save", "Ctrl+S")) {}
                if (ImGui::MenuItem("Save As..")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Examples")) {
                static bool showAppMainMenuBar = false;
                static bool showAppConsole     = false;
                static bool showAppLog         = false;
                ImGui::MenuItem("Main menu bar", NULL, &showAppMainMenuBar);
                ImGui::MenuItem("Console", NULL, &showAppConsole);
                ImGui::MenuItem("Log", NULL, &showAppLog);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Tools")) {
                static bool showAppMetrics     = false;
                static bool showAppStyleEditor = false;
                static bool showAppAbout       = false;
                ImGui::MenuItem("Metrics/Debugger", NULL, &showAppMetrics);
                ImGui::MenuItem("Style Editor", NULL, &showAppStyleEditor);
                ImGui::MenuItem("About PGE", NULL, &showAppAbout);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        ImGui::End();
    }
};

class EditorObjectsPanel {
    bool m_isOpen;

public:
    void
    View()
    {
        ImGuiWindowFlags windowFlags = 0;
        windowFlags |= ImGuiWindowFlags_MenuBar;
        windowFlags |= ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoCollapse;
        windowFlags |= ImGuiWindowFlags_NoNav;

        const int menuBarHeight = 20;
        const int panelWidth    = 250;
        const int panelHeight   = 720 - menuBarHeight;
        ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight));
        static bool once = false;
        if (!once) {
            once = true;
            ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));
        }
        if (!ImGui::Begin("Objects", nullptr, windowFlags)) {
            ImGui::End();
            return;
        }

        if (ImGui::TreeNode("Scene layout")) {
            static bool isCubeSelected = false;
            if (ImGui::TreeNodeEx("Cube", ImGuiTreeNodeFlags_Bullet | (ImGuiTreeNodeFlags_Selected * isCubeSelected))) {
                isCubeSelected = true;
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }


        ImGui::End();
    }
};

class Editor {
    EditorMenu         m_menu;
    EditorObjectsPanel m_objectsPanel;

public:
    void
    View(int displayWidth, int displayHeight)
    {
        m_menu.View(displayWidth);
        m_objectsPanel.View();
    }
};


int
main()
{
    pge::os_DisplayWin32         display("PGE Editor", 1280, 720, WindowProc);
    pge::gfx_GraphicsAdapterD3D11 adapter(display.GetWidth(), display.GetHeight());
    pge::gfx_GraphicsDevice graphics(&adapter);
    ImGui_Init(display, adapter);

    Editor editor;

    while (!display.IsCloseRequested()) {
        display.HandleEvents();

        graphics.Clear();
        ImGui_NewFrame();
        editor.View(display.GetWidth(), display.GetHeight());
        ImGui_EndFrame();
        graphics.Present();
    }

    return 0;
}