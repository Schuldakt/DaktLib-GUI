// ============================================================================
// DaktLib GUI Module - DrawList
// ============================================================================
// Immediate-mode draw command list for rendering shapes, text, and images.
// ============================================================================

#pragma once

#include <dakt/core/Buffer.hpp>
#include <dakt/gui/Types.hpp>

#include <functional>
#include <span>
#include <vector>

namespace dakt::gui
{

// ============================================================================
// Vertex Format
// ============================================================================

struct DrawVertex
{
    Vec2 pos;
    Vec2 uv;
    u32 color;  // ABGR format for D3D11 compatibility

    DrawVertex() = default;
    DrawVertex(Vec2 p, Vec2 t, Color c) : pos(p), uv(t), color(c.toABGR()) {}
};

using DrawIndex = u32;

// ============================================================================
// Draw Command
// ============================================================================

struct DrawCmd
{
    u32 indexCount = 0;                                // Number of indices to draw
    u32 indexOffset = 0;                               // Offset into index buffer
    Rect clipRect;                                     // Scissor rectangle
    uintptr_t textureId = 0;                           // Texture handle (0 = white texture)
    std::function<void(const DrawCmd&)> userCallback;  // Optional custom callback

    [[nodiscard]] bool hasCallback() const { return userCallback != nullptr; }
};

// ============================================================================
// Path Builder (for complex shapes)
// ============================================================================

class PathBuilder
{
public:
    void clear() { m_points.clear(); }
    void moveTo(Vec2 p)
    {
        m_points.clear();
        m_points.push_back(p);
    }
    void lineTo(Vec2 p) { m_points.push_back(p); }
    void close();

    void arcTo(Vec2 center, f32 radius, f32 startAngle, f32 endAngle, i32 segments = 0);
    void bezierCurveTo(Vec2 c1, Vec2 c2, Vec2 end, i32 segments = 0);
    void quadraticCurveTo(Vec2 control, Vec2 end, i32 segments = 0);

    [[nodiscard]] std::span<const Vec2> points() const { return m_points; }
    [[nodiscard]] bool empty() const { return m_points.empty(); }
    [[nodiscard]] usize size() const { return m_points.size(); }

private:
    std::vector<Vec2> m_points;
};

// ============================================================================
// DrawList
// ============================================================================

class DrawList
{
public:
    DrawList();
    ~DrawList() = default;

    // Reset for new frame
    void clear();

    // Clipping
    void pushClipRect(Rect rect, bool intersectWithCurrent = true);
    void pushClipRectFullScreen();
    void popClipRect();
    [[nodiscard]] Rect getCurrentClipRect() const;

    // Primitives - Lines
    void addLine(Vec2 a, Vec2 b, Color color, f32 thickness = 1.0f);
    void addPolyline(std::span<const Vec2> points, Color color, bool closed, f32 thickness = 1.0f);

    // Primitives - Rectangles
    void addRect(Rect rect, Color color, f32 rounding = 0.0f, Corner corners = Corner::All, f32 thickness = 1.0f);
    void addRectFilled(Rect rect, Color color, f32 rounding = 0.0f, Corner corners = Corner::All);
    void addRectFilledMultiColor(Rect rect, Color topLeft, Color topRight, Color bottomLeft, Color bottomRight);

    // Primitives - Circles
    void addCircle(Vec2 center, f32 radius, Color color, i32 segments = 0, f32 thickness = 1.0f);
    void addCircleFilled(Vec2 center, f32 radius, Color color, i32 segments = 0);
    void addEllipse(Vec2 center, Vec2 radii, Color color, i32 segments = 0, f32 thickness = 1.0f);
    void addEllipseFilled(Vec2 center, Vec2 radii, Color color, i32 segments = 0);

    // Primitives - Triangles
    void addTriangle(Vec2 a, Vec2 b, Vec2 c, Color color, f32 thickness = 1.0f);
    void addTriangleFilled(Vec2 a, Vec2 b, Vec2 c, Color color);

    // Primitives - Polygons
    void addConvexPolyFilled(std::span<const Vec2> points, Color color);
    void addConcavePolyFilled(std::span<const Vec2> points, Color color);

    // Primitives - Hexagons
    void addHexagon(Hexagon hex, Color color, f32 thickness = 1.0f);
    void addHexagonFilled(Hexagon hex, Color color);

    // Primitives - Arcs and Beziers
    void addArc(Vec2 center, f32 radius, f32 startAngle, f32 endAngle, Color color, i32 segments = 0,
                f32 thickness = 1.0f);
    void addArcFilled(Vec2 center, f32 radius, f32 startAngle, f32 endAngle, Color color, i32 segments = 0);
    void addBezierCubic(Vec2 p0, Vec2 c0, Vec2 c1, Vec2 p1, Color color, f32 thickness = 1.0f, i32 segments = 0);
    void addBezierQuadratic(Vec2 p0, Vec2 c, Vec2 p1, Color color, f32 thickness = 1.0f, i32 segments = 0);

    // Text (requires font texture)
    void addText(Vec2 pos, StringView text, Color color, f32 fontSize = 14.0f);
    void addTextEx(Rect bounds, StringView text, Color color, f32 fontSize, TextAlign align = TextAlign::topLeft(),
                   bool wrap = false);

    // Images
    void addImage(uintptr_t textureId, Rect rect, Color tint = Color::white());
    void addImage(uintptr_t textureId, Rect rect, Vec2 uvMin, Vec2 uvMax, Color tint = Color::white());
    void addImageRounded(uintptr_t textureId, Rect rect, Vec2 uvMin, Vec2 uvMax, Color tint, f32 rounding,
                         Corner corners = Corner::All);
    void addImageQuad(uintptr_t textureId, Vec2 a, Vec2 b, Vec2 c, Vec2 d, Vec2 uvA = {0, 0}, Vec2 uvB = {1, 0},
                      Vec2 uvC = {1, 1}, Vec2 uvD = {0, 1}, Color tint = Color::white());

    // Path-based drawing
    PathBuilder& pathBuilder() { return m_pathBuilder; }
    void pathStroke(Color color, bool closed, f32 thickness = 1.0f);
    void pathFill(Color color);
    void pathFillConvex(Color color);

    // Gradient fills
    void addRectFilledGradientH(Rect rect, Color left, Color right, f32 rounding = 0.0f);
    void addRectFilledGradientV(Rect rect, Color top, Color bottom, f32 rounding = 0.0f);

    // Command splitting (for different textures/shaders)
    void addCallback(std::function<void(const DrawCmd&)> callback);
    void addDrawCmd();  // Force new command

    // Access draw data
    [[nodiscard]] std::span<const DrawVertex> vertices() const { return m_vertices; }
    [[nodiscard]] std::span<const DrawIndex> indices() const { return m_indices; }
    [[nodiscard]] std::span<const DrawCmd> commands() const { return m_commands; }

    [[nodiscard]] u32 vertexCount() const { return static_cast<u32>(m_vertices.size()); }
    [[nodiscard]] u32 indexCount() const { return static_cast<u32>(m_indices.size()); }
    [[nodiscard]] u32 commandCount() const { return static_cast<u32>(m_commands.size()); }

    // Settings
    void setAntiAliasedLines(bool enabled) { m_antiAliasedLines = enabled; }
    void setAntiAliasedFill(bool enabled) { m_antiAliasedFill = enabled; }
    void setCurveTessellationTol(f32 tol) { m_curveTessellationTol = tol; }
    void setCircleSegmentMaxError(f32 err) { m_circleSegmentMaxError = err; }

private:
    // Internal vertex/index manipulation
    void reserveVertices(usize count);
    void reserveIndices(usize count);

    DrawVertex* allocVertices(u32 count);
    DrawIndex* allocIndices(u32 count);

    void primRect(Vec2 a, Vec2 c, u32 color);
    void primRectUV(Vec2 a, Vec2 c, Vec2 uvA, Vec2 uvC, u32 color);
    void primQuadUV(Vec2 a, Vec2 b, Vec2 c, Vec2 d, Vec2 uvA, Vec2 uvB, Vec2 uvC, Vec2 uvD, u32 color);

    void addPolylineInternal(std::span<const Vec2> points, u32 color, bool closed, f32 thickness);
    void addConvexPolyFilledInternal(std::span<const Vec2> points, u32 color);

    i32 calcCircleAutoSegmentCount(f32 radius) const;
    i32 calcArcAutoSegmentCount(f32 radius, f32 arcLength) const;

    void updateClipRect();

private:
    std::vector<DrawVertex> m_vertices;
    std::vector<DrawIndex> m_indices;
    std::vector<DrawCmd> m_commands;
    std::vector<Rect> m_clipRectStack;

    PathBuilder m_pathBuilder;

    uintptr_t m_currentTextureId = 0;
    Vec2 m_whitePixelUV = {0.5f, 0.5f};  // UV for solid color rendering

    // Anti-aliasing settings
    bool m_antiAliasedLines = true;
    bool m_antiAliasedFill = true;
    f32 m_curveTessellationTol = 1.25f;
    f32 m_circleSegmentMaxError = 0.3f;

    // Precomputed circle segments for common radii
    static constexpr i32 kCircleSegmentMin = 4;
    static constexpr i32 kCircleSegmentMax = 512;
};

// ============================================================================
// DrawListSplitter (for layered drawing)
// ============================================================================

class DrawListSplitter
{
public:
    void clear();
    void split(DrawList* drawList, i32 channelCount);
    void merge(DrawList* drawList);
    void setCurrentChannel(DrawList* drawList, i32 channel);

    [[nodiscard]] i32 channelCount() const { return m_channelCount; }
    [[nodiscard]] i32 currentChannel() const { return m_currentChannel; }

private:
    struct Channel
    {
        std::vector<DrawCmd> commands;
        std::vector<DrawIndex> indices;
    };

    std::vector<Channel> m_channels;
    i32 m_channelCount = 0;
    i32 m_currentChannel = 0;
};

}  // namespace dakt::gui
