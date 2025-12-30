// ============================================================================
// DaktLib GUI Module - DrawList Implementation
// ============================================================================

#include <dakt/gui/core/DrawList.hpp>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace dakt::gui
{

constexpr f32 PI = std::numbers::pi_v<f32>;
constexpr f32 PI2 = PI * 2.0f;

// ============================================================================
// PathBuilder
// ============================================================================

void PathBuilder::close()
{
    if (m_points.size() >= 2)
    {
        if (m_points.front() != m_points.back())
        {
            m_points.push_back(m_points.front());
        }
    }
}

void PathBuilder::arcTo(Vec2 center, f32 radius, f32 startAngle, f32 endAngle, i32 segments)
{
    if (segments <= 0)
    {
        f32 arcLength = std::abs(endAngle - startAngle) * radius;
        segments = std::max(4, static_cast<i32>(arcLength / 2.0f));
    }

    for (i32 i = 0; i <= segments; ++i)
    {
        f32 t = static_cast<f32>(i) / static_cast<f32>(segments);
        f32 angle = startAngle + (endAngle - startAngle) * t;
        m_points.push_back(Vec2{center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius});
    }
}

void PathBuilder::bezierCurveTo(Vec2 c1, Vec2 c2, Vec2 end, i32 segments)
{
    if (m_points.empty())
        return;

    Vec2 start = m_points.back();
    if (segments <= 0)
        segments = 12;

    for (i32 i = 1; i <= segments; ++i)
    {
        f32 t = static_cast<f32>(i) / static_cast<f32>(segments);
        f32 u = 1.0f - t;
        f32 tt = t * t;
        f32 uu = u * u;
        f32 uuu = uu * u;
        f32 ttt = tt * t;

        Vec2 p = start * uuu + c1 * (3.0f * uu * t) + c2 * (3.0f * u * tt) + end * ttt;
        m_points.push_back(p);
    }
}

void PathBuilder::quadraticCurveTo(Vec2 control, Vec2 end, i32 segments)
{
    if (m_points.empty())
        return;

    Vec2 start = m_points.back();
    if (segments <= 0)
        segments = 8;

    for (i32 i = 1; i <= segments; ++i)
    {
        f32 t = static_cast<f32>(i) / static_cast<f32>(segments);
        f32 u = 1.0f - t;

        Vec2 p = start * (u * u) + control * (2.0f * u * t) + end * (t * t);
        m_points.push_back(p);
    }
}

// ============================================================================
// DrawList
// ============================================================================

DrawList::DrawList()
{
    clear();
}

void DrawList::clear()
{
    m_vertices.clear();
    m_indices.clear();
    m_commands.clear();
    m_clipRectStack.clear();
    m_pathBuilder.clear();
    m_currentTextureId = 0;

    // Add default command
    m_commands.push_back(DrawCmd{});
    m_commands.back().clipRect = Rect{0, 0, 8192, 8192};
}

void DrawList::pushClipRect(Rect rect, bool intersectWithCurrent)
{
    if (intersectWithCurrent && !m_clipRectStack.empty())
    {
        rect = rect.intersection(m_clipRectStack.back());
    }
    m_clipRectStack.push_back(rect);
    updateClipRect();
}

void DrawList::pushClipRectFullScreen()
{
    pushClipRect(Rect{0, 0, 8192, 8192}, false);
}

void DrawList::popClipRect()
{
    if (!m_clipRectStack.empty())
    {
        m_clipRectStack.pop_back();
    }
    updateClipRect();
}

Rect DrawList::getCurrentClipRect() const
{
    if (m_clipRectStack.empty())
    {
        return Rect{0, 0, 8192, 8192};
    }
    return m_clipRectStack.back();
}

void DrawList::updateClipRect()
{
    Rect clipRect = getCurrentClipRect();

    if (m_commands.empty() || m_commands.back().indexCount != 0)
    {
        m_commands.push_back(DrawCmd{});
    }
    m_commands.back().clipRect = clipRect;
}

void DrawList::reserveVertices(usize count)
{
    m_vertices.reserve(m_vertices.size() + count);
}

void DrawList::reserveIndices(usize count)
{
    m_indices.reserve(m_indices.size() + count);
}

DrawVertex* DrawList::allocVertices(u32 count)
{
    usize idx = m_vertices.size();
    m_vertices.resize(idx + count);
    return &m_vertices[idx];
}

DrawIndex* DrawList::allocIndices(u32 count)
{
    usize idx = m_indices.size();
    m_indices.resize(idx + count);
    m_commands.back().indexCount += count;
    return &m_indices[idx];
}

i32 DrawList::calcCircleAutoSegmentCount(f32 radius) const
{
    f32 maxError = m_circleSegmentMaxError;
    i32 segments = static_cast<i32>(std::ceil(PI2 / std::acos(1.0f - std::min(maxError, radius) / radius)));
    return std::clamp(segments, kCircleSegmentMin, kCircleSegmentMax);
}

i32 DrawList::calcArcAutoSegmentCount(f32 radius, f32 arcLength) const
{
    f32 segmentLength = PI2 * radius / static_cast<f32>(calcCircleAutoSegmentCount(radius));
    return std::max(1, static_cast<i32>(std::ceil(arcLength / segmentLength)));
}

// ============================================================================
// Primitive Drawing
// ============================================================================

void DrawList::primRect(Vec2 a, Vec2 c, u32 color)
{
    Vec2 b{c.x, a.y};
    Vec2 d{a.x, c.y};

    DrawIndex idx = static_cast<DrawIndex>(m_vertices.size());

    auto* vtx = allocVertices(4);
    vtx[0] = DrawVertex{a, m_whitePixelUV, Color::fromU32(color)};
    vtx[1] = DrawVertex{b, m_whitePixelUV, Color::fromU32(color)};
    vtx[2] = DrawVertex{c, m_whitePixelUV, Color::fromU32(color)};
    vtx[3] = DrawVertex{d, m_whitePixelUV, Color::fromU32(color)};

    auto* ix = allocIndices(6);
    ix[0] = idx;
    ix[1] = idx + 1;
    ix[2] = idx + 2;
    ix[3] = idx;
    ix[4] = idx + 2;
    ix[5] = idx + 3;
}

void DrawList::primRectUV(Vec2 a, Vec2 c, Vec2 uvA, Vec2 uvC, u32 color)
{
    Vec2 b{c.x, a.y};
    Vec2 d{a.x, c.y};
    Vec2 uvB{uvC.x, uvA.y};
    Vec2 uvD{uvA.x, uvC.y};

    DrawIndex idx = static_cast<DrawIndex>(m_vertices.size());

    auto* vtx = allocVertices(4);
    vtx[0] = DrawVertex{a, uvA, Color::fromU32(color)};
    vtx[1] = DrawVertex{b, uvB, Color::fromU32(color)};
    vtx[2] = DrawVertex{c, uvC, Color::fromU32(color)};
    vtx[3] = DrawVertex{d, uvD, Color::fromU32(color)};

    auto* ix = allocIndices(6);
    ix[0] = idx;
    ix[1] = idx + 1;
    ix[2] = idx + 2;
    ix[3] = idx;
    ix[4] = idx + 2;
    ix[5] = idx + 3;
}

void DrawList::primQuadUV(Vec2 a, Vec2 b, Vec2 c, Vec2 d, Vec2 uvA, Vec2 uvB, Vec2 uvC, Vec2 uvD, u32 color)
{
    DrawIndex idx = static_cast<DrawIndex>(m_vertices.size());

    auto* vtx = allocVertices(4);
    vtx[0] = DrawVertex{a, uvA, Color::fromU32(color)};
    vtx[1] = DrawVertex{b, uvB, Color::fromU32(color)};
    vtx[2] = DrawVertex{c, uvC, Color::fromU32(color)};
    vtx[3] = DrawVertex{d, uvD, Color::fromU32(color)};

    auto* ix = allocIndices(6);
    ix[0] = idx;
    ix[1] = idx + 1;
    ix[2] = idx + 2;
    ix[3] = idx;
    ix[4] = idx + 2;
    ix[5] = idx + 3;
}

// ============================================================================
// Line Drawing
// ============================================================================

void DrawList::addLine(Vec2 a, Vec2 b, Color color, f32 thickness)
{
    if (color.a == 0)
        return;

    Vec2 dir = (b - a).normalized();
    Vec2 normal = dir.perpendicular() * (thickness * 0.5f);

    DrawIndex idx = static_cast<DrawIndex>(m_vertices.size());

    auto* vtx = allocVertices(4);
    vtx[0] = DrawVertex{a + normal, m_whitePixelUV, color};
    vtx[1] = DrawVertex{b + normal, m_whitePixelUV, color};
    vtx[2] = DrawVertex{b - normal, m_whitePixelUV, color};
    vtx[3] = DrawVertex{a - normal, m_whitePixelUV, color};

    auto* ix = allocIndices(6);
    ix[0] = idx;
    ix[1] = idx + 1;
    ix[2] = idx + 2;
    ix[3] = idx;
    ix[4] = idx + 2;
    ix[5] = idx + 3;
}

void DrawList::addPolyline(std::span<const Vec2> points, Color color, bool closed, f32 thickness)
{
    if (points.size() < 2 || color.a == 0)
        return;
    addPolylineInternal(points, color.toU32(), closed, thickness);
}

void DrawList::addPolylineInternal(std::span<const Vec2> points, u32 color, bool closed, f32 thickness)
{
    usize count = points.size();
    if (count < 2)
        return;

    f32 halfThickness = thickness * 0.5f;

    for (usize i = 0; i < count - (closed ? 0 : 1); ++i)
    {
        Vec2 p0 = points[i];
        Vec2 p1 = points[(i + 1) % count];

        Vec2 dir = (p1 - p0).normalized();
        Vec2 normal = dir.perpendicular() * halfThickness;

        DrawIndex idx = static_cast<DrawIndex>(m_vertices.size());

        auto* vtx = allocVertices(4);
        vtx[0] = DrawVertex{p0 + normal, m_whitePixelUV, Color::fromU32(color)};
        vtx[1] = DrawVertex{p1 + normal, m_whitePixelUV, Color::fromU32(color)};
        vtx[2] = DrawVertex{p1 - normal, m_whitePixelUV, Color::fromU32(color)};
        vtx[3] = DrawVertex{p0 - normal, m_whitePixelUV, Color::fromU32(color)};

        auto* ix = allocIndices(6);
        ix[0] = idx;
        ix[1] = idx + 1;
        ix[2] = idx + 2;
        ix[3] = idx;
        ix[4] = idx + 2;
        ix[5] = idx + 3;
    }
}

// ============================================================================
// Rectangle Drawing
// ============================================================================

void DrawList::addRect(Rect rect, Color color, f32 rounding, Corner corners, f32 thickness)
{
    if (color.a == 0 || !rect.isValid())
        return;

    if (rounding <= 0.0f || corners == Corner::None)
    {
        // Simple rectangle outline
        std::array<Vec2, 4> pts = {rect.topLeft(), rect.topRight(), rect.bottomRight(), rect.bottomLeft()};
        addPolyline(pts, color, true, thickness);
    }
    else
    {
        // Rounded rectangle outline
        m_pathBuilder.clear();

        f32 r = std::min(rounding, std::min(rect.width(), rect.height()) * 0.5f);

        if (hasFlag(corners, Corner::TopLeft))
        {
            m_pathBuilder.arcTo({rect.left() + r, rect.top() + r}, r, PI, PI * 1.5f, 4);
        }
        else
        {
            m_pathBuilder.moveTo(rect.topLeft());
        }

        if (hasFlag(corners, Corner::TopRight))
        {
            m_pathBuilder.arcTo({rect.right() - r, rect.top() + r}, r, PI * 1.5f, PI2, 4);
        }
        else
        {
            m_pathBuilder.lineTo(rect.topRight());
        }

        if (hasFlag(corners, Corner::BottomRight))
        {
            m_pathBuilder.arcTo({rect.right() - r, rect.bottom() - r}, r, 0, PI * 0.5f, 4);
        }
        else
        {
            m_pathBuilder.lineTo(rect.bottomRight());
        }

        if (hasFlag(corners, Corner::BottomLeft))
        {
            m_pathBuilder.arcTo({rect.left() + r, rect.bottom() - r}, r, PI * 0.5f, PI, 4);
        }
        else
        {
            m_pathBuilder.lineTo(rect.bottomLeft());
        }

        pathStroke(color, true, thickness);
    }
}

void DrawList::addRectFilled(Rect rect, Color color, f32 rounding, Corner corners)
{
    if (color.a == 0 || !rect.isValid())
        return;

    if (rounding <= 0.0f || corners == Corner::None)
    {
        primRect(rect.min, rect.max, color.toU32());
    }
    else
    {
        // Rounded filled rectangle
        m_pathBuilder.clear();

        f32 r = std::min(rounding, std::min(rect.width(), rect.height()) * 0.5f);

        if (hasFlag(corners, Corner::TopLeft))
        {
            m_pathBuilder.arcTo({rect.left() + r, rect.top() + r}, r, PI, PI * 1.5f, 4);
        }
        else
        {
            m_pathBuilder.moveTo(rect.topLeft());
        }

        if (hasFlag(corners, Corner::TopRight))
        {
            m_pathBuilder.arcTo({rect.right() - r, rect.top() + r}, r, PI * 1.5f, PI2, 4);
        }
        else
        {
            m_pathBuilder.lineTo(rect.topRight());
        }

        if (hasFlag(corners, Corner::BottomRight))
        {
            m_pathBuilder.arcTo({rect.right() - r, rect.bottom() - r}, r, 0, PI * 0.5f, 4);
        }
        else
        {
            m_pathBuilder.lineTo(rect.bottomRight());
        }

        if (hasFlag(corners, Corner::BottomLeft))
        {
            m_pathBuilder.arcTo({rect.left() + r, rect.bottom() - r}, r, PI * 0.5f, PI, 4);
        }
        else
        {
            m_pathBuilder.lineTo(rect.bottomLeft());
        }

        pathFillConvex(color);
    }
}

void DrawList::addRectFilledMultiColor(Rect rect, Color topLeft, Color topRight, Color bottomLeft, Color bottomRight)
{
    if (!rect.isValid())
        return;

    DrawIndex idx = static_cast<DrawIndex>(m_vertices.size());

    auto* vtx = allocVertices(4);
    vtx[0] = DrawVertex{rect.topLeft(), m_whitePixelUV, topLeft};
    vtx[1] = DrawVertex{rect.topRight(), m_whitePixelUV, topRight};
    vtx[2] = DrawVertex{rect.bottomRight(), m_whitePixelUV, bottomRight};
    vtx[3] = DrawVertex{rect.bottomLeft(), m_whitePixelUV, bottomLeft};

    auto* ix = allocIndices(6);
    ix[0] = idx;
    ix[1] = idx + 1;
    ix[2] = idx + 2;
    ix[3] = idx;
    ix[4] = idx + 2;
    ix[5] = idx + 3;
}

// ============================================================================
// Circle Drawing
// ============================================================================

void DrawList::addCircle(Vec2 center, f32 radius, Color color, i32 segments, f32 thickness)
{
    if (color.a == 0 || radius <= 0.0f)
        return;

    if (segments <= 0)
        segments = calcCircleAutoSegmentCount(radius);

    std::vector<Vec2> points(segments);
    for (i32 i = 0; i < segments; ++i)
    {
        f32 angle = PI2 * static_cast<f32>(i) / static_cast<f32>(segments);
        points[i] = Vec2{center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius};
    }

    addPolyline(points, color, true, thickness);
}

void DrawList::addCircleFilled(Vec2 center, f32 radius, Color color, i32 segments)
{
    if (color.a == 0 || radius <= 0.0f)
        return;

    if (segments <= 0)
        segments = calcCircleAutoSegmentCount(radius);

    std::vector<Vec2> points(segments);
    for (i32 i = 0; i < segments; ++i)
    {
        f32 angle = PI2 * static_cast<f32>(i) / static_cast<f32>(segments);
        points[i] = Vec2{center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius};
    }

    addConvexPolyFilled(points, color);
}

void DrawList::addEllipse(Vec2 center, Vec2 radii, Color color, i32 segments, f32 thickness)
{
    if (color.a == 0 || radii.x <= 0.0f || radii.y <= 0.0f)
        return;

    if (segments <= 0)
        segments = calcCircleAutoSegmentCount(std::max(radii.x, radii.y));

    std::vector<Vec2> points(segments);
    for (i32 i = 0; i < segments; ++i)
    {
        f32 angle = PI2 * static_cast<f32>(i) / static_cast<f32>(segments);
        points[i] = Vec2{center.x + std::cos(angle) * radii.x, center.y + std::sin(angle) * radii.y};
    }

    addPolyline(points, color, true, thickness);
}

void DrawList::addEllipseFilled(Vec2 center, Vec2 radii, Color color, i32 segments)
{
    if (color.a == 0 || radii.x <= 0.0f || radii.y <= 0.0f)
        return;

    if (segments <= 0)
        segments = calcCircleAutoSegmentCount(std::max(radii.x, radii.y));

    std::vector<Vec2> points(segments);
    for (i32 i = 0; i < segments; ++i)
    {
        f32 angle = PI2 * static_cast<f32>(i) / static_cast<f32>(segments);
        points[i] = Vec2{center.x + std::cos(angle) * radii.x, center.y + std::sin(angle) * radii.y};
    }

    addConvexPolyFilled(points, color);
}

// ============================================================================
// Triangle Drawing
// ============================================================================

void DrawList::addTriangle(Vec2 a, Vec2 b, Vec2 c, Color color, f32 thickness)
{
    if (color.a == 0)
        return;
    std::array<Vec2, 3> pts = {a, b, c};
    addPolyline(pts, color, true, thickness);
}

void DrawList::addTriangleFilled(Vec2 a, Vec2 b, Vec2 c, Color color)
{
    if (color.a == 0)
        return;

    DrawIndex idx = static_cast<DrawIndex>(m_vertices.size());

    auto* vtx = allocVertices(3);
    vtx[0] = DrawVertex{a, m_whitePixelUV, color};
    vtx[1] = DrawVertex{b, m_whitePixelUV, color};
    vtx[2] = DrawVertex{c, m_whitePixelUV, color};

    auto* ix = allocIndices(3);
    ix[0] = idx;
    ix[1] = idx + 1;
    ix[2] = idx + 2;
}

// ============================================================================
// Polygon Drawing
// ============================================================================

void DrawList::addConvexPolyFilled(std::span<const Vec2> points, Color color)
{
    if (points.size() < 3 || color.a == 0)
        return;
    addConvexPolyFilledInternal(points, color.toU32());
}

void DrawList::addConvexPolyFilledInternal(std::span<const Vec2> points, u32 color)
{
    usize count = points.size();
    if (count < 3)
        return;

    DrawIndex idx = static_cast<DrawIndex>(m_vertices.size());

    auto* vtx = allocVertices(static_cast<u32>(count));
    for (usize i = 0; i < count; ++i)
    {
        vtx[i] = DrawVertex{points[i], m_whitePixelUV, Color::fromU32(color)};
    }

    // Fan triangulation for convex polygon
    auto* ix = allocIndices(static_cast<u32>((count - 2) * 3));
    for (usize i = 0; i < count - 2; ++i)
    {
        ix[i * 3 + 0] = idx;
        ix[i * 3 + 1] = idx + static_cast<DrawIndex>(i + 1);
        ix[i * 3 + 2] = idx + static_cast<DrawIndex>(i + 2);
    }
}

void DrawList::addConcavePolyFilled(std::span<const Vec2> points, Color color)
{
    // TODO: Implement ear-clipping or other triangulation for concave polygons
    // For now, fall back to convex (may produce incorrect results for concave)
    addConvexPolyFilled(points, color);
}

// ============================================================================
// Hexagon Drawing
// ============================================================================

void DrawList::addHexagon(Hexagon hex, Color color, f32 thickness)
{
    if (color.a == 0 || hex.radius <= 0.0f)
        return;

    auto verts = hex.vertices();
    addPolyline(verts, color, true, thickness);
}

void DrawList::addHexagonFilled(Hexagon hex, Color color)
{
    if (color.a == 0 || hex.radius <= 0.0f)
        return;

    auto verts = hex.vertices();
    addConvexPolyFilled(verts, color);
}

// ============================================================================
// Arc and Bezier Drawing
// ============================================================================

void DrawList::addArc(Vec2 center, f32 radius, f32 startAngle, f32 endAngle, Color color, i32 segments, f32 thickness)
{
    if (color.a == 0 || radius <= 0.0f)
        return;

    f32 arcLength = std::abs(endAngle - startAngle) * radius;
    if (segments <= 0)
        segments = calcArcAutoSegmentCount(radius, arcLength);

    std::vector<Vec2> points(segments + 1);
    for (i32 i = 0; i <= segments; ++i)
    {
        f32 t = static_cast<f32>(i) / static_cast<f32>(segments);
        f32 angle = startAngle + (endAngle - startAngle) * t;
        points[i] = Vec2{center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius};
    }

    addPolyline(points, color, false, thickness);
}

void DrawList::addArcFilled(Vec2 center, f32 radius, f32 startAngle, f32 endAngle, Color color, i32 segments)
{
    if (color.a == 0 || radius <= 0.0f)
        return;

    f32 arcLength = std::abs(endAngle - startAngle) * radius;
    if (segments <= 0)
        segments = calcArcAutoSegmentCount(radius, arcLength);

    std::vector<Vec2> points(segments + 2);
    points[0] = center;

    for (i32 i = 0; i <= segments; ++i)
    {
        f32 t = static_cast<f32>(i) / static_cast<f32>(segments);
        f32 angle = startAngle + (endAngle - startAngle) * t;
        points[i + 1] = Vec2{center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius};
    }

    addConvexPolyFilled(points, color);
}

void DrawList::addBezierCubic(Vec2 p0, Vec2 c0, Vec2 c1, Vec2 p1, Color color, f32 thickness, i32 segments)
{
    if (color.a == 0)
        return;
    if (segments <= 0)
        segments = 12;

    std::vector<Vec2> points(segments + 1);
    for (i32 i = 0; i <= segments; ++i)
    {
        f32 t = static_cast<f32>(i) / static_cast<f32>(segments);
        f32 u = 1.0f - t;
        f32 tt = t * t;
        f32 uu = u * u;
        f32 uuu = uu * u;
        f32 ttt = tt * t;

        points[i] = p0 * uuu + c0 * (3.0f * uu * t) + c1 * (3.0f * u * tt) + p1 * ttt;
    }

    addPolyline(points, color, false, thickness);
}

void DrawList::addBezierQuadratic(Vec2 p0, Vec2 c, Vec2 p1, Color color, f32 thickness, i32 segments)
{
    if (color.a == 0)
        return;
    if (segments <= 0)
        segments = 8;

    std::vector<Vec2> points(segments + 1);
    for (i32 i = 0; i <= segments; ++i)
    {
        f32 t = static_cast<f32>(i) / static_cast<f32>(segments);
        f32 u = 1.0f - t;

        points[i] = p0 * (u * u) + c * (2.0f * u * t) + p1 * (t * t);
    }

    addPolyline(points, color, false, thickness);
}

// ============================================================================
// Text Drawing (stub - requires font system)
// ============================================================================

void DrawList::addText(Vec2 pos, StringView text, Color color, f32 fontSize)
{
    // TODO: Implement with Font class
    (void)pos;
    (void)text;
    (void)color;
    (void)fontSize;
}

void DrawList::addTextEx(Rect bounds, StringView text, Color color, f32 fontSize, TextAlign align, bool wrap)
{
    // TODO: Implement with Font class
    (void)bounds;
    (void)text;
    (void)color;
    (void)fontSize;
    (void)align;
    (void)wrap;
}

// ============================================================================
// Image Drawing
// ============================================================================

void DrawList::addImage(uintptr_t textureId, Rect rect, Color tint)
{
    addImage(textureId, rect, {0, 0}, {1, 1}, tint);
}

void DrawList::addImage(uintptr_t textureId, Rect rect, Vec2 uvMin, Vec2 uvMax, Color tint)
{
    if (tint.a == 0 || !rect.isValid())
        return;

    // Switch texture if needed
    if (textureId != m_currentTextureId)
    {
        addDrawCmd();
        m_currentTextureId = textureId;
        m_commands.back().textureId = textureId;
    }

    primRectUV(rect.min, rect.max, uvMin, uvMax, tint.toU32());
}

void DrawList::addImageRounded(uintptr_t textureId, Rect rect, Vec2 uvMin, Vec2 uvMax, Color tint, f32 rounding,
                               Corner corners)
{
    if (tint.a == 0 || !rect.isValid())
        return;

    if (rounding <= 0.0f || corners == Corner::None)
    {
        addImage(textureId, rect, uvMin, uvMax, tint);
        return;
    }

    // TODO: Implement rounded image with UV mapping
    // For now, fall back to regular image
    addImage(textureId, rect, uvMin, uvMax, tint);
}

void DrawList::addImageQuad(uintptr_t textureId, Vec2 a, Vec2 b, Vec2 c, Vec2 d, Vec2 uvA, Vec2 uvB, Vec2 uvC, Vec2 uvD,
                            Color tint)
{
    if (tint.a == 0)
        return;

    if (textureId != m_currentTextureId)
    {
        addDrawCmd();
        m_currentTextureId = textureId;
        m_commands.back().textureId = textureId;
    }

    primQuadUV(a, b, c, d, uvA, uvB, uvC, uvD, tint.toU32());
}

// ============================================================================
// Path Operations
// ============================================================================

void DrawList::pathStroke(Color color, bool closed, f32 thickness)
{
    if (m_pathBuilder.empty() || color.a == 0)
    {
        m_pathBuilder.clear();
        return;
    }

    addPolyline(m_pathBuilder.points(), color, closed, thickness);
    m_pathBuilder.clear();
}

void DrawList::pathFill(Color color)
{
    if (m_pathBuilder.size() < 3 || color.a == 0)
    {
        m_pathBuilder.clear();
        return;
    }

    addConcavePolyFilled(m_pathBuilder.points(), color);
    m_pathBuilder.clear();
}

void DrawList::pathFillConvex(Color color)
{
    if (m_pathBuilder.size() < 3 || color.a == 0)
    {
        m_pathBuilder.clear();
        return;
    }

    addConvexPolyFilled(m_pathBuilder.points(), color);
    m_pathBuilder.clear();
}

// ============================================================================
// Gradient Fills
// ============================================================================

void DrawList::addRectFilledGradientH(Rect rect, Color left, Color right, f32 rounding)
{
    if (!rect.isValid())
        return;

    if (rounding <= 0.0f)
    {
        addRectFilledMultiColor(rect, left, right, left, right);
    }
    else
    {
        // TODO: Implement rounded gradient
        addRectFilledMultiColor(rect, left, right, left, right);
    }
}

void DrawList::addRectFilledGradientV(Rect rect, Color top, Color bottom, f32 rounding)
{
    if (!rect.isValid())
        return;

    if (rounding <= 0.0f)
    {
        addRectFilledMultiColor(rect, top, top, bottom, bottom);
    }
    else
    {
        // TODO: Implement rounded gradient
        addRectFilledMultiColor(rect, top, top, bottom, bottom);
    }
}

// ============================================================================
// Command Management
// ============================================================================

void DrawList::addCallback(std::function<void(const DrawCmd&)> callback)
{
    addDrawCmd();
    m_commands.back().userCallback = std::move(callback);
    addDrawCmd();  // Ensure next draw commands go to a new cmd
}

void DrawList::addDrawCmd()
{
    DrawCmd cmd;
    cmd.clipRect = getCurrentClipRect();
    cmd.textureId = m_currentTextureId;
    cmd.indexOffset = static_cast<u32>(m_indices.size());
    m_commands.push_back(cmd);
}

// ============================================================================
// DrawListSplitter
// ============================================================================

void DrawListSplitter::clear()
{
    m_channels.clear();
    m_channelCount = 0;
    m_currentChannel = 0;
}

void DrawListSplitter::split(DrawList* drawList, i32 channelCount)
{
    (void)drawList;
    m_channelCount = channelCount;
    m_channels.resize(channelCount);
    m_currentChannel = 0;

    for (auto& channel : m_channels)
    {
        channel.commands.clear();
        channel.indices.clear();
    }
}

void DrawListSplitter::merge(DrawList* drawList)
{
    (void)drawList;
    // TODO: Merge all channels back into main drawList
    clear();
}

void DrawListSplitter::setCurrentChannel(DrawList* drawList, i32 channel)
{
    (void)drawList;
    if (channel >= 0 && channel < m_channelCount)
    {
        m_currentChannel = channel;
    }
}

}  // namespace dakt::gui
