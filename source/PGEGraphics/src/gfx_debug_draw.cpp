#include "../include/gfx_debug_draw.h"
#include "../include/gfx_vertex_layout.h"
#include "../include/gfx_shader.h"
#include "../include/gfx_buffer.h"
#include "../include/gfx_graphics_device.h"
#include "../include/gfx_texture.h"

namespace pge
{
    struct DebugPoint {
        math_Vec3 position;
        math_Vec3 color;
        float     size;
    };

    static const unsigned s_verticesPerPoint = 6;
    static unsigned       s_pointCount       = 0;
    static const unsigned s_pointCapacity    = 1000;
    static DebugPoint     s_points[s_pointCapacity];

    static unsigned       s_pointDepthCount    = 0;
    static const unsigned s_pointDepthCapacity = s_pointCapacity;
    static DebugPoint     s_pointsDepth[s_pointDepthCapacity];


    struct DebugLine {
        math_Vec4 beginTransformed;
        math_Vec4 endTransformed;
        math_Vec4 color;
        float     width;
    };

    static const unsigned s_verticesPerLine = 6;
    static unsigned       s_lineCount       = 0;
    static const unsigned s_lineCapacity    = 50000;
    static DebugLine      s_lines[s_lineCapacity];

    static unsigned       s_lineDepthCount    = 0;
    static const unsigned s_lineDepthCapacity = s_lineCapacity;
    static DebugLine      s_linesDepth[s_lineDepthCapacity];


    struct DebugBillboard {
        math_Vec3            position;
        math_Vec2            size;
        const gfx_Texture2D* texture;
        math_Vec3            color;
    };

    static const unsigned s_billboardCapacity = 1000;
    static DebugBillboard s_billboards[s_billboardCapacity];
    static unsigned       s_billboardCount = 0;

    struct DebugBillboardVertex {
        math_Vec4 position;
        math_Vec2 size;
    };
    static gfx_VertexAttribute s_debugBillboardVertexAttribs[]
        = {gfx_VertexAttribute("POSITION", gfx_VertexAttributeType::FLOAT4), gfx_VertexAttribute("SIZE", gfx_VertexAttributeType::FLOAT2)};
    static const unsigned       s_debugBillboardVertexCapacity = s_verticesPerPoint * s_billboardCapacity;
    static DebugBillboardVertex s_debugBillboardVertices[s_debugBillboardVertexCapacity];

    static const char* s_debugBillboardVertexShaderSource = "cbuffer cbuffer_transforms : register(b0)"
                                                            "{"
                                                            "  row_major float4x4 viewMatrix;"
                                                            "  row_major float4x4 projMatrix;"
                                                            "};"
                                                            ""
                                                            "struct VertexIn"
                                                            "{"
                                                            "  float4 position : POSITION;"
                                                            "  float2 size     : SIZE;"
                                                            "};"
                                                            ""
                                                            "struct GeomIn"
                                                            "{"
                                                            "  float4 position : POSITION;"
                                                            "  float2 size     : SIZE;"
                                                            "};"
                                                            ""
                                                            "GeomIn "
                                                            "VSMain(VertexIn vertex)"
                                                            "{"
                                                            "  GeomIn output;"
                                                            "  output.position = vertex.position;"
                                                            "  output.size     = vertex.size;"
                                                            "  return output;"
                                                            "}";


    static const char* s_debugBillboardGeometryShaderSource = "cbuffer cbuffer_transforms : register(b0)"
                                                              "{"
                                                              "  row_major float4x4 viewMatrix;"
                                                              "  row_major float4x4 projMatrix;"
                                                              "};"
                                                              ""
                                                              "struct GeomIn {"
                                                              "  float4 position : POSITION;"
                                                              "  float2 size     : SIZE;"
                                                              "};"
                                                              ""
                                                              "struct PixelIn"
                                                              "{"
                                                              "  float4 positionNDC : SV_POSITION;"
                                                              "  float2 texcoord    : TEXTURECOORD;"
                                                              "};"
                                                              ""
                                                              "[maxvertexcount(6)]"
                                                              "void GSMain(point GeomIn input[1], inout TriangleStream<PixelIn> outputStream)"
                                                              "{"
                                                              "    float hw = input[0].size.x / 2;"
                                                              "    float hh = input[0].size.y / 2;"

                                                              "    float4 viewPos = mul(viewMatrix, input[0].position);"

                                                              "    PixelIn topLeft;"
                                                              "    topLeft.positionNDC = mul(projMatrix, viewPos + float4(-hw,hh,0,0));"
                                                              "    topLeft.texcoord = float2(0, 0);"

                                                              "    PixelIn bottomLeft;"
                                                              "    bottomLeft.positionNDC = mul(projMatrix, viewPos + float4(-hw,-hh,0,0));"
                                                              "    bottomLeft.texcoord = float2(0, 1);"

                                                              "    PixelIn bottomRight;"
                                                              "    bottomRight.positionNDC = mul(projMatrix, viewPos + float4(hw,-hh,0,0));"
                                                              "    bottomRight.texcoord = float2(1, 1);"

                                                              "    PixelIn topRight;"
                                                              "    topRight.positionNDC = mul(projMatrix, viewPos + float4(hw,hh,0,0));"
                                                              "    topRight.texcoord = float2(1, 0);"

                                                              "    outputStream.Append(topLeft);"
                                                              "    outputStream.Append(bottomLeft);"
                                                              "    outputStream.Append(bottomRight);"

                                                              "    outputStream.RestartStrip();"

                                                              "    outputStream.Append(bottomRight);"
                                                              "    outputStream.Append(topRight);"
                                                              "    outputStream.Append(topLeft);"

                                                              // "    outputStream.RestartStrip();"
                                                              "}";


    static const char* s_debugVertexShaderSource = "cbuffer cbuffer_transforms : register(b0)"
                                                   "{"
                                                   "  row_major float4x4 viewMatrix;"
                                                   "  row_major float4x4 projMatrix;"
                                                   "};"
                                                   ""
                                                   "struct vertex_data"
                                                   "{"
                                                   "  float4 position : POSITION;"
                                                   "  float4 color    : COLOR;"
                                                   "};"
                                                   ""
                                                   "struct pixel_shader_input"
                                                   "{"
                                                   "  float4 PositionNDC : SV_POSITION;"
                                                   "  float4 color       : PS_COLOR;"
                                                   "};"
                                                   ""
                                                   "pixel_shader_input "
                                                   "VSMain(vertex_data Vertex)"
                                                   "{"
                                                   "  pixel_shader_input Output;"
                                                   "  Output.PositionNDC = mul(projMatrix, Vertex.position);"
                                                   "  Output.color       = Vertex.color;"
                                                   "  return Output;"
                                                   "}";

    static const char* s_debugPixelShaderSource = "struct pixel_shader_input"
                                                  "{"
                                                  "  float4 PositionNDC : SV_POSITION;"
                                                  "  float4 color       : PS_COLOR;"
                                                  "};"
                                                  ""
                                                  "float4 "
                                                  "PSMain(pixel_shader_input Input) : SV_TARGET"
                                                  "{"
                                                  "  return Input.color;"
                                                  "}";

    static const char* s_debugPixelTexShaderSource = "Texture2D DiffuseMap        : register(t0);"
                                                     "SamplerState DiffuseSampler {};"
//                                                     "BlendState Blend {"
//                                                     "  SrcBlend              = D3D11_BLEND_ZERO;"
//                                                     "  DestBlend             = D3D11_BLEND_ONE;"
//                                                     "  BlendOp               = D3D11_BLEND_OP_ADD;"
//                                                     "  SrcBlendAlpha         = D3D11_BLEND_ONE;"
//                                                     "  DestBlendAlpha        = D3D11_BLEND_ZERO;"
//                                                     "  BlendOpAlpha          = D3D11_BLEND_OP_ADD;"
//                                                     "  RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;"
//                                                     "};"
                                                     ""
                                                     "struct PixelIn"
                                                     "{"
                                                     "  float4 positionNDC : SV_POSITION;"
                                                     "  float2 texcoord    : TEXTURECOORD;"
                                                     "};"
                                                     ""
                                                     "float4 "
                                                     "PSMain(PixelIn input) : SV_TARGET"
                                                     "{"
                                                     "  return DiffuseMap.Sample(DiffuseSampler, input.texcoord);"
                                                     "}";

    struct DebugCBTransforms {
        math_Mat4x4 viewMatrix;
        math_Mat4x4 projMatrix;
    };

    struct DebugVertex {
        math_Vec4 position;
        math_Vec4 color;
    };

    static gfx_VertexAttribute s_debugVertexAttribs[]
        = {gfx_VertexAttribute("POSITION", gfx_VertexAttributeType::FLOAT4), gfx_VertexAttribute("COLOR", gfx_VertexAttributeType::FLOAT4)};

    static const unsigned s_debugVertexCapacity = s_verticesPerPoint * s_pointCapacity + s_verticesPerLine * s_lineCapacity;
    static DebugVertex    s_debugVertices[s_debugVertexCapacity];

    static const unsigned s_debugVerticesDepthCapacity = s_verticesPerPoint * s_pointDepthCapacity + s_verticesPerLine * s_lineDepthCapacity;
    static DebugVertex    s_debugVerticesDepth[s_debugVerticesDepthCapacity];

    static DebugCBTransforms s_debugTransforms;

    struct DebugDrawResources {
        gfx_VertexLayout     vertexLayout;
        gfx_VertexLayout     billboardVertexLayout;
        gfx_VertexShader     vertexShader;
        gfx_VertexShader     billVertexShader;
        gfx_PixelShader      colorPixelShader;
        gfx_PixelShader      texPixelShader;
        gfx_GeometryShader   billGeomShader;
        gfx_VertexBuffer     vertexBuffer;
        gfx_VertexBuffer     billVertexBuffer;
        gfx_ConstantBuffer   transformsCB;
        gfx_GraphicsDevice*  graphicsDevice;
        gfx_GraphicsAdapter* graphicsAdapter;

        DebugDrawResources(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice)
            : vertexLayout(graphicsAdapter, s_debugVertexAttribs, sizeof(s_debugVertexAttribs) / sizeof(gfx_VertexAttribute))
            , billboardVertexLayout(graphicsAdapter,
                                    s_debugBillboardVertexAttribs,
                                    sizeof(s_debugBillboardVertexAttribs) / sizeof(gfx_VertexAttribute))
            , vertexShader(graphicsAdapter, s_debugVertexShaderSource, strlen(s_debugVertexShaderSource))
            , billVertexShader(graphicsAdapter, s_debugBillboardVertexShaderSource, strlen(s_debugBillboardVertexShaderSource))
            , colorPixelShader(graphicsAdapter, s_debugPixelShaderSource, strlen(s_debugPixelShaderSource))
            , texPixelShader(graphicsAdapter, s_debugPixelTexShaderSource, strlen(s_debugPixelTexShaderSource))
            , billGeomShader(graphicsAdapter, s_debugBillboardGeometryShaderSource, strlen(s_debugBillboardGeometryShaderSource))
            , vertexBuffer(graphicsAdapter, nullptr, sizeof(s_debugVertices), gfx_BufferUsage::DYNAMIC)
            , billVertexBuffer(graphicsAdapter, nullptr, sizeof(s_debugBillboardVertices), gfx_BufferUsage::DYNAMIC)
            , transformsCB(graphicsAdapter, nullptr, sizeof(s_debugTransforms), gfx_BufferUsage::DYNAMIC)
            , graphicsDevice(graphicsDevice)
            , graphicsAdapter(graphicsAdapter)
        {}
    };
    static DebugDrawResources* s_resources = nullptr;

    void
    gfx_DebugDraw_Initialize(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice)
    {
        if (s_resources == nullptr) {
            s_resources = new DebugDrawResources(graphicsAdapter, graphicsDevice);
        }
    }

    void
    gfx_DebugDraw_Shutdown()
    {
        if (s_resources != nullptr) {
            delete s_resources;
        }
    }

    void
    gfx_DebugDraw_SetView(const math_Mat4x4& viewMatrix)
    {
        s_debugTransforms.viewMatrix = viewMatrix;
    }

    void
    gfx_DebugDraw_SetProjection(const math_Mat4x4& projectionMatrix)
    {
        s_debugTransforms.projMatrix = projectionMatrix;
    }

    void
    gfx_DebugDraw_Point(const math_Vec3& position, const math_Vec3& color, float size, bool depthTest)
    {
        DebugPoint point;
        point.position = position;
        point.color    = color;
        point.size     = size;
        if (depthTest) {
            s_pointsDepth[s_pointDepthCount++] = point;
            if (s_pointDepthCount >= s_pointDepthCapacity) {
                gfx_DebugDraw_Flush();
            }
        } else {
            s_points[s_pointCount++] = point;
            if (s_pointCount >= s_pointCapacity) {
                gfx_DebugDraw_Flush();
            }
        }
    }

    void
    gfx_DebugDraw_Line(const math_Vec3& begin, const math_Vec3& end, const math_Vec3& color, float width, bool depthTest)
    {
        DebugLine line;
        line.beginTransformed = s_debugTransforms.viewMatrix * math_Vec4(begin, 1.f);
        line.endTransformed   = s_debugTransforms.viewMatrix * math_Vec4(end, 1.f);
        line.color            = math_Vec4(color, 1.f);
        line.width            = width;
        if (depthTest) {
            s_linesDepth[s_lineDepthCount++] = line;
            if (s_lineDepthCount >= s_lineDepthCapacity) {
                gfx_DebugDraw_Flush();
            }
        } else {
            s_lines[s_lineCount++] = line;
            if (s_lineCount >= s_lineCapacity) {
                gfx_DebugDraw_Flush();
            }
        }
    }

    void
    gfx_DebugDraw_Box(const math_Vec3& min, const math_Vec3& max, const math_Vec3& color, float lineWidth, bool depthTest)
    {
        math_Vec3 p[8];
        p[0] = math_Vec3(min.x, min.y, min.z);
        p[1] = math_Vec3(max.x, min.y, min.z);
        p[2] = math_Vec3(max.x, max.y, min.z);
        p[3] = math_Vec3(min.x, max.y, min.z);

        p[4] = math_Vec3(min.x, min.y, max.z);
        p[5] = math_Vec3(max.x, min.y, max.z);
        p[6] = math_Vec3(max.x, max.y, max.z);
        p[7] = math_Vec3(min.x, max.y, max.z);

        for (int i = 0; i < 4; ++i) {
            gfx_DebugDraw_Line(p[i], p[(i + 1) % 4], color, lineWidth, depthTest);
        }
        for (int i = 0; i < 4; ++i) {
            gfx_DebugDraw_Line(p[4 + i], p[4 + ((i + 1) % 4)], color, lineWidth, depthTest);
        }
        for (int i = 0; i < 4; ++i) {
            gfx_DebugDraw_Line(p[i], p[i + 4], color, lineWidth, depthTest);
        }
    }

    void
    gfx_DebugDraw_GridXY(const math_Vec3& origin,
                         const float      lineLength,
                         const math_Vec2& cellSize,
                         const math_Vec3& color,
                         const float      lineWidth,
                         bool             depthTest)
    {
        int stepsX = (int)(lineLength / cellSize.x);
        int stepsY = (int)(lineLength / cellSize.y);
        for (int i = -stepsX / 2; i < stepsX / 2; ++i) {
            gfx_DebugDraw_Line(origin + math_Vec3(i * cellSize.x, -lineLength / 2, 0.f),
                               origin + math_Vec3(i * cellSize.x, lineLength / 2, 0.f),
                               color,
                               lineWidth,
                               depthTest);
        }
        for (int i = -stepsY / 2; i < stepsY / 2; ++i) {
            gfx_DebugDraw_Line(origin + math_Vec3(-lineLength / 2, i * cellSize.y, 0.f),
                               origin + math_Vec3(lineLength / 2, i * cellSize.y, 0.f),
                               color,
                               lineWidth,
                               depthTest);
        }
    }

    void
    gfx_DebugDraw_Billboard(const math_Vec3& position, const math_Vec2& size, const gfx_Texture2D* texture, const math_Vec3& color)
    {
        core_Assert(s_billboardCount < s_billboardCapacity);
        DebugBillboard billboard;
        billboard.position               = position;
        billboard.size                   = size;
        billboard.texture                = texture;
        billboard.color                  = color;
        s_billboards[s_billboardCount++] = billboard;
        if (s_billboardCount >= s_billboardCapacity) {
            gfx_DebugDraw_Flush();
        }
    }

    /**
     * @brief Calculates vertices in modelview space for s_points.
     *
     * @param destination The output buffer.
     * @param pointsBuffer The s_points to generate the vertices for.
     * @param count The amount of s_points.
     * @return The index of the next vertex in the output buffer.
     */
    static unsigned
    CalculateVerticesForPoints(DebugVertex* destination, DebugPoint* pointsBuffer, unsigned count)
    {
        // NOTE: Instead of extracting these from the s_debugViewMatrix, we
        //       set them as constants. Since we want the primitives to always
        //       face the camera.
        const math_Vec3 viewRight   = math_Vec3(1.f, 0.f, 0.f);
        const math_Vec3 viewUp      = math_Vec3(0.f, 1.f, 0.f);
        const math_Vec3 viewForward = math_Vec3(0.f, 0.f, -1.f);

        unsigned vertexIndex = 0;
        for (unsigned i = 0; i < count; ++i) {
            const math_Vec3& position            = pointsBuffer[i].position;
            const math_Vec3  positionTransformed = (s_debugTransforms.viewMatrix * math_Vec4(position, 1.f)).xyz;
            math_Vec3        positions[4];
            positions[0] = positionTransformed + pointsBuffer[i].size * (-viewRight + viewUp);
            positions[1] = positionTransformed + pointsBuffer[i].size * (-viewRight - viewUp);
            positions[2] = positionTransformed + pointsBuffer[i].size * (viewRight - viewUp);
            positions[3] = positionTransformed + pointsBuffer[i].size * (viewRight + viewUp);

            size_t indices[] = { 0, 1, 2, 2, 3, 0 };
            for (size_t j = 0; j < 6; ++j) {
                const size_t& index = indices[j];
                destination[vertexIndex].position = math_Vec4(positions[index], 1.0f);
                destination[vertexIndex].color = math_Vec4(pointsBuffer[i].color, 1.0f);
                ++vertexIndex;
            }
        }
        return vertexIndex;
    }

    /**
     * @brief Calculates the vertices in modelview space for s_lines.
     *
     * @param destination The output buffer.
     * @param linesBuffer The s_lines to generate the vertices for.
     * @param s_lineCount The amount of s_lines.
     * @return The index of the next vertex in the output buffer.
     */
    static unsigned
    CalculateVerticesForLines(DebugVertex* destination, DebugLine* linesBuffer, unsigned count)
    {
        unsigned vertexIndex = 0;
        for (unsigned i = 0; i < count; ++i) {
            const math_Vec3 beginTransformed = linesBuffer[i].beginTransformed.xyz;
            const math_Vec3 endTransformed   = linesBuffer[i].endTransformed.xyz;
            if (beginTransformed == endTransformed)
                continue;

            const math_Vec3 lineVec = math_Normalize(endTransformed - beginTransformed);
            const math_Vec3 midLine = beginTransformed + 0.5f * (endTransformed - beginTransformed);
            math_Vec3       side    = math_Cross(lineVec, midLine);
            if (math_FloatEqual(lineVec.z, 0) || math_LengthSquared(side) == 0) {
                side = (math_Mat4x4(
               // clang-format off
                    0, -1, 0, 0,
                    1, 0, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1
                // clang-format on
                ) * math_Vec4(lineVec, 1)).xyz;
           }
            const math_Vec3 sideVec = math_Normalize(side);
            const float     lineHW  = linesBuffer[i].width * 0.5f;

            math_Vec4 positions[4];
            positions[0] = math_Vec4(beginTransformed + sideVec * lineHW, 1.f);
            positions[1] = math_Vec4(beginTransformed - sideVec * lineHW, 1.f);
            positions[2] = math_Vec4(endTransformed - sideVec * lineHW, 1.f);
            positions[3] = math_Vec4(endTransformed + sideVec * lineHW, 1.f);

            size_t indices[] = { 0, 1, 2, 2, 3, 0 };
            for (size_t j = 0; j < 6; ++j) {
                const size_t& index = indices[j];
                destination[vertexIndex].position = positions[index];
                destination[vertexIndex].color = linesBuffer[i].color;
                ++vertexIndex;
            }
        }
        return vertexIndex;
    }


    void
    gfx_DebugDraw_Flush()
    {
        unsigned vertexIndex = 0;

        vertexIndex = CalculateVerticesForPoints(&s_debugVertices[0], &s_points[0], s_pointCount);
        CalculateVerticesForLines(&s_debugVertices[vertexIndex], &s_lines[0], s_lineCount);

        vertexIndex = CalculateVerticesForPoints(&s_debugVerticesDepth[0], &s_pointsDepth[0], s_pointDepthCount);
        CalculateVerticesForLines(&s_debugVerticesDepth[vertexIndex], &s_linesDepth[0], s_lineDepthCount);

        s_resources->transformsCB.Update(&s_debugTransforms, sizeof(s_debugTransforms));

        s_resources->vertexLayout.Bind();
        s_resources->vertexShader.Bind();
        s_resources->colorPixelShader.Bind();
        s_resources->vertexBuffer.Bind(0, sizeof(DebugVertex), 0);
        s_resources->transformsCB.BindVS(0);

        // Draw without depth.
        // TODO: Create depth state resources at init so we can use both
        //        if (s_pointCount + s_lineCount > 0) {
        //            GPU_SetDepthMode(DEPTH_MODE::NONE);
        //            GPU_UpdateBuffer(DebugVertexBuffer, s_debugVertices, sizeof(s_debugVertices), 0);
        //            // Draw s_points.
        //            if (s_pointCount > 0) {
        //                GPU_Draw(0, s_verticesPerPoint * s_pointCount);
        //            }
        //            // Draw s_lines.
        //            if (s_lineCount > 0) {
        //                GPU_Draw(s_verticesPerPoint * s_pointCount, s_verticesPerLine * s_lineCount);
        //            }
        //        }

        // Draw with depth.
        if (s_pointDepthCount + s_lineDepthCount > 0) {
            //            GPU_SetDepthMode(DEPTH_MODE::LESS_OR_EQUAL);
            s_resources->vertexBuffer.Update(s_debugVerticesDepth, sizeof(s_debugVerticesDepth), 0);

            // Draw points.
            if (s_pointDepthCount > 0) {
                s_resources->graphicsDevice->Draw(gfx_PrimitiveType::TRIANGLELIST, 0, s_verticesPerPoint * s_pointDepthCount);
            }
            // Draw lines.
            if (s_lineDepthCount > 0) {
                s_resources->graphicsDevice->Draw(gfx_PrimitiveType::TRIANGLELIST,
                                                  s_verticesPerPoint * s_pointDepthCount,
                                                  s_verticesPerLine * s_lineDepthCount);
            }
        }


        // Draw billboards
        if (s_billboardCount > 0) {
            s_resources->billboardVertexLayout.Bind();
            s_resources->billVertexShader.Bind();
            s_resources->billGeomShader.Bind();
            s_resources->texPixelShader.Bind();

            for (size_t i = 0; i < s_billboardCount; ++i) {
                s_debugBillboardVertices[i].position = math_Vec4(s_billboards[i].position, 1);
                s_debugBillboardVertices[i].size     = s_billboards[i].size;
            }
            s_resources->billVertexBuffer.Update(s_debugBillboardVertices, sizeof(s_debugBillboardVertices), 0);
            s_resources->billVertexBuffer.Bind(0, sizeof(DebugBillboardVertex), 0);

            s_resources->transformsCB.BindVS(0);
            s_resources->transformsCB.BindGS(0);

            const gfx_Texture2D* lastTex      = s_billboards[0].texture;
            size_t               sameTexCount = 1;
            size_t               offset       = 0;
            for (size_t i = 1; i < s_billboardCount; ++i) {
                if (s_billboards[i].texture != lastTex) {
                    lastTex->Bind(0);
                    s_resources->graphicsDevice->Draw(gfx_PrimitiveType::POINTLIST, offset, sameTexCount);
                    offset += sameTexCount;
                    lastTex = s_billboards[i].texture;
                } else {
                    sameTexCount++;
                }
            }
            lastTex->Bind(0);
            s_resources->graphicsDevice->Draw(gfx_PrimitiveType::POINTLIST, offset, sameTexCount);
        }
        gfx_GeometryShader_Unbind(s_resources->graphicsAdapter);

        // TODO: Push/Pop or Get previous state so we don't have to remember.
        // GPU_SetDepthMode(DEPTH_MODE::LESS_OR_EQUAL);

        s_pointCount = s_pointDepthCount = 0;
        s_lineCount = s_lineDepthCount = 0;
        s_billboardCount               = 0;
    }

} // namespace pge