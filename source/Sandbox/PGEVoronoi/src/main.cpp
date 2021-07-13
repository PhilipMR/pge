#include <core_display_win32.h>
#include <gfx_graphics_adapter_d3d11.h>
#include <gfx_graphics_device.h>
#include <gfx_render_target.h>
#include <gfx_debug_draw.h>
#include <input_events_win32.h>
#include <vector>

using namespace pge;

static LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_DESTROY)
        PostQuitMessage(0);
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

struct VoronoiRegion {
    math_Vec2              point;
    math_Vec3              color;
    std::vector<math_Vec2> polygonPoints;
};

struct VoronoiMap {
    float                      width;
    float                      height;
    std::vector<VoronoiRegion> regions;
};

VoronoiMap
GenerateVoronoiMap(float width, float height, int n)
{
    VoronoiMap map;
    map.width  = width;
    map.height = height;
    srand(GetTickCount());

    // Generate points and colors
    for (int i = 0; i < n; ++i) {
        float px = (width / 1000.0f) * (rand() % 1000);
        float py = (height / 1000.0f) * (rand() % 1000);

        VoronoiRegion region;
        region.point = math_Vec2(px, py);
        region.color = math_Vec3(px / width, py / height, 1.0f);
        map.regions.push_back(region);
    }

    // Generate region polygons
    struct Line {
        math_Vec2 from;
        math_Vec2 to;
    };
    auto linesIntersect = [](const Line& lineA, const Line& lineB) {
        // https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
        const float& x1  = lineA.from.x;
        const float& y1  = lineA.from.y;
        const float& x2  = lineA.to.x;
        const float& y2  = lineA.to.y;
        const float& x3  = lineB.from.x;
        const float& y3  = lineB.from.y;
        const float& x4  = lineB.to.x;
        const float& y4  = lineB.to.y;
        const float  det = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
        if (math_FloatEqual(det, 0)) {
            return std::make_pair(false, math_Vec2::Zero());
        }
        const float px = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / det;
        const float py = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / det;
        return std::make_pair(true, math_Vec2(px, py));
    };


    Line mapBorders[4];
    mapBorders[0].from = math_Vec2::Zero();
    mapBorders[0].to   = math_Vec2(map.width, 0);

    mapBorders[1].from = math_Vec2::Zero();
    mapBorders[1].to   = math_Vec2(0, map.height);

    mapBorders[2].from = math_Vec2(map.width, 0);
    mapBorders[2].to   = math_Vec2(map.width, map.height);

    mapBorders[3].from = math_Vec2(0, map.height);
    mapBorders[3].to   = math_Vec2(map.width, map.height);

    for (int i = 0; i < n; ++i) {
        const math_Vec2& point   = map.regions[i].point;
        auto&            polygon = map.regions[i].polygonPoints;
        for (int j = i + 1; j < n; ++j) {
            const math_Vec2& otherPoint = map.regions[j].point;
            const math_Vec2  diff       = otherPoint - point;
            const math_Vec2  diffDir    = math_Normalize(diff);
            const math_Vec2  diffOrtho(-diffDir.y, diffDir.x); // Rotate diffDir by 90 degrees
            const math_Vec2  middle = point + diff * 0.5f;

            Line orthoLine;
            orthoLine.from = middle;
            orthoLine.to   = diffOrtho;
            for (size_t k = 0; k < 4; ++k) {
                auto intersect = linesIntersect(orthoLine, mapBorders[k]);
                if (intersect.first) {
                    polygon.push_back(intersect.second);
                }

                orthoLine.to = -orthoLine.to;
                intersect    = linesIntersect(orthoLine, mapBorders[k]);
                if (intersect.first) {
                    polygon.push_back(intersect.second);
                }
            }
        }
    }

    return map;
}

void
DrawVoronoiMap(const VoronoiMap& map)
{
    float psize = (map.width / 5.0f) * 0.01f;
    for (const auto& region : map.regions) {
        gfx_DebugDraw_Point(math_Vec3(region.point.x, region.point.y, 0), region.color, psize);
    }
}

int
main()
{
    const math_Vec2 resolution(1600, 900);
    const float     aspect = resolution.x / resolution.y;

    core_DisplayWin32        display("PGE Voronoi", resolution.x, resolution.y, WindowProc);
    gfx_GraphicsAdapterD3D11 graphicsAdapter(display.GetWindowHandle(), display.GetWidth(), display.GetHeight());
    gfx_GraphicsDevice       graphicsDevice(&graphicsAdapter);

    gfx_DebugDraw_Initialize(&graphicsAdapter, &graphicsDevice);


    const math_Vec2 viewSize(5.0f * aspect, 5.0f);
    math_Mat4x4     viewMatrix = math_Mat4x4::Identity();
    math_Mat4x4     projMatrix = math_OrthographicRH(viewSize.x, viewSize.y, 0.1f, 1000.0f);

    VoronoiMap map = GenerateVoronoiMap(viewSize.x, viewSize.y, 2);

    while (!display.IsCloseRequested()) {
        display.HandleEvents();

        gfx_DebugDraw_SetView(viewMatrix);
        gfx_DebugDraw_SetProjection(projMatrix);
        gfx_RenderTarget_ClearMainRTV(&graphicsAdapter);

        DrawVoronoiMap(map);
        gfx_DebugDraw_Render();

        graphicsDevice.Present();
    }
    gfx_DebugDraw_Shutdown();
    return 0;
}