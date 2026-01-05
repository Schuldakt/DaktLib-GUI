#include "dakt/gui/draw/DrawList.hpp"
#include <algorithm>
#include <cmath>

namespace dakt::gui {

// Constants
constexpr float PI = 3.14159265358979323846f;

DrawList::DrawList() {
    // Reserve reasonable initial capacity
    vertices_.reserve(4096);
    indices_.reserve(8192);
    commands_.reserve(64);

    // Default clip rect (will be set to window size)
    currentClipRect_ = Rect(0, 0, 10000, 10000);
}

DrawList::~DrawList() = default;

void DrawList::reset() {
    vertices_.clear();
    indices_.clear();
    commands_.clear();
    clipRectStack_.clear();
    currentTexture_ = 0;
    currentClipRect_ = Rect(0, 0, 10000, 10000);
}

void DrawList::addCommand(DrawCommandType type, uint32_t vertexCount, uint32_t indexCount) {
    // Try to merge with previous command if compatible
    if (!commands_.empty() && type == DrawCommandType::DrawTriangles) {
        auto& prev = commands_.back();
        if (prev.type == DrawCommandType::DrawTriangles && prev.clipRect == currentClipRect_ && prev.textureID == currentTexture_) {
            // Extend previous command
            prev.vertexCount += vertexCount;
            prev.indexCount += indexCount;
            return;
        }
    }

    DrawCommand cmd;
    cmd.type = type;
    cmd.vertexOffset = static_cast<uint32_t>(vertices_.size()) - vertexCount;
    cmd.vertexCount = vertexCount;
    cmd.indexOffset = static_cast<uint32_t>(indices_.size()) - indexCount;
    cmd.indexCount = indexCount;
    cmd.clipRect = currentClipRect_;
    cmd.textureID = currentTexture_;
    commands_.push_back(cmd);
}

void DrawList::reserveVertices(size_t count) {
    if (vertices_.capacity() < vertices_.size() + count) {
        vertices_.reserve(vertices_.size() + count + 1024);
    }
}

void DrawList::reserveIndices(size_t count) {
    if (indices_.capacity() < indices_.size() + count) {
        indices_.reserve(indices_.size() + count + 2048);
    }
}

void DrawList::addVertex(const Vertex& vertex) { vertices_.push_back(vertex); }

void DrawList::addIndex(uint32_t index) { indices_.push_back(index); }

void DrawList::addTriangleIndices(uint32_t i0, uint32_t i1, uint32_t i2) {
    indices_.push_back(i0);
    indices_.push_back(i1);
    indices_.push_back(i2);
}

void DrawList::drawRect(const Rect& rect, Color color) {
    // Draw rectangle outline (4 lines)
    float thickness = 1.0f;

    // Top
    drawLine(Vec2(rect.x, rect.y), Vec2(rect.right(), rect.y), color, thickness);
    // Right
    drawLine(Vec2(rect.right(), rect.y), Vec2(rect.right(), rect.bottom()), color, thickness);
    // Bottom
    drawLine(Vec2(rect.right(), rect.bottom()), Vec2(rect.x, rect.bottom()), color, thickness);
    // Left
    drawLine(Vec2(rect.x, rect.bottom()), Vec2(rect.x, rect.y), color, thickness);
}

void DrawList::drawRectFilled(const Rect& rect, Color color) {
    reserveVertices(4);
    reserveIndices(6);

    uint32_t baseIdx = static_cast<uint32_t>(vertices_.size());

    // Add 4 vertices (corners)
    Vec2 uv(0.0f, 0.0f);                                                       // Default UV for solid colors
    vertices_.push_back(Vertex(Vec2(rect.x, rect.y), uv, color));              // Top-left
    vertices_.push_back(Vertex(Vec2(rect.right(), rect.y), uv, color));        // Top-right
    vertices_.push_back(Vertex(Vec2(rect.right(), rect.bottom()), uv, color)); // Bottom-right
    vertices_.push_back(Vertex(Vec2(rect.x, rect.bottom()), uv, color));       // Bottom-left

    // Add 2 triangles (6 indices)
    addTriangleIndices(baseIdx + 0, baseIdx + 1, baseIdx + 2);
    addTriangleIndices(baseIdx + 0, baseIdx + 2, baseIdx + 3);

    addCommand(DrawCommandType::DrawTriangles, 4, 6);
}

void DrawList::drawRectRounded(const Rect& rect, Color color, float radius) {
    BorderRadius br(radius);

    // Clamp radii to half of smallest dimension
    float maxRadius = std::min(rect.width, rect.height) / 2.0f;
    float tl = std::min(br.topLeft, maxRadius);
    float tr = std::min(br.topRight, maxRadius);
    float bra = std::min(br.bottomRight, maxRadius);
    float bl = std::min(br.bottomLeft, maxRadius);

    if (tl <= 0 && tr <= 0 && bra <= 0 && bl <= 0) {
        drawRect(rect, color);
        return;
    }

    constexpr int SEGMENTS_PER_CORNER = 8;

    // Build path points for the rounded rectangle outline
    std::vector<Vec2> points;
    points.reserve(4 * (SEGMENTS_PER_CORNER + 1));

    auto addCornerArc = [&](Vec2 center, float r, float startAngle) {
        for (int i = 0; i <= SEGMENTS_PER_CORNER; ++i) {
            float angle = startAngle + (PI / 2.0f) * (float(i) / SEGMENTS_PER_CORNER);
            points.push_back(center + Vec2(std::cos(angle) * r, std::sin(angle) * r));
        }
    };

    // Top-left corner (PI to 3PI/2)
    addCornerArc(Vec2(rect.x + tl, rect.y + tl), tl, PI);
    // Top-right corner (3PI/2 to 2PI)
    addCornerArc(Vec2(rect.right() - tr, rect.y + tr), tr, 3.0f * PI / 2.0f);
    // Bottom-right corner (0 to PI/2)
    addCornerArc(Vec2(rect.right() - bra, rect.bottom() - bra), bra, 0.0f);
    // Bottom-left corner (PI/2 to PI)
    addCornerArc(Vec2(rect.x + bl, rect.bottom() - bl), bl, PI / 2.0f);

    // Draw lines connecting all points
    for (size_t i = 0; i < points.size(); ++i) {
        size_t next = (i + 1) % points.size();
        drawLine(points[i], points[next], color, 1.0f);
    }
}

void DrawList::drawRectFilledRounded(const Rect& rect, Color color, float radius) { drawRectFilledRounded(rect, color, BorderRadius(radius)); }

void DrawList::drawRectFilledRounded(const Rect& rect, Color color, const BorderRadius& radius) {
    // Clamp radii to half of smallest dimension
    float maxRadius = std::min(rect.width, rect.height) / 2.0f;
    float tl = std::min(radius.topLeft, maxRadius);
    float tr = std::min(radius.topRight, maxRadius);
    float br = std::min(radius.bottomRight, maxRadius);
    float bl = std::min(radius.bottomLeft, maxRadius);

    if (tl <= 0 && tr <= 0 && br <= 0 && bl <= 0) {
        drawRectFilled(rect, color);
        return;
    }

    // Build rounded rectangle with arc segments
    constexpr int SEGMENTS_PER_CORNER = 8;
    int totalVertices = 4 + SEGMENTS_PER_CORNER * 4;

    reserveVertices(totalVertices + 1); // +1 for center
    reserveIndices(totalVertices * 3);

    uint32_t baseIdx = static_cast<uint32_t>(vertices_.size());
    Vec2 uv(0.0f, 0.0f);

    // Center point for fan triangulation
    Vec2 center = rect.center();
    vertices_.push_back(Vertex(center, uv, color));

    // Add corner arc vertices
    auto addCorner = [&](Vec2 cornerCenter, float r, float startAngle) {
        for (int i = 0; i <= SEGMENTS_PER_CORNER; ++i) {
            float angle = startAngle + (PI / 2.0f) * (float(i) / SEGMENTS_PER_CORNER);
            Vec2 pos = cornerCenter + Vec2(std::cos(angle) * r, std::sin(angle) * r);
            vertices_.push_back(Vertex(pos, uv, color));
        }
    };

    // Top-left corner (PI to 3PI/2)
    addCorner(Vec2(rect.x + tl, rect.y + tl), tl, PI);
    // Top-right corner (3PI/2 to 2PI)
    addCorner(Vec2(rect.right() - tr, rect.y + tr), tr, 3.0f * PI / 2.0f);
    // Bottom-right corner (0 to PI/2)
    addCorner(Vec2(rect.right() - br, rect.bottom() - br), br, 0.0f);
    // Bottom-left corner (PI/2 to PI)
    addCorner(Vec2(rect.x + bl, rect.bottom() - bl), bl, PI / 2.0f);

    // Create triangle fan from center
    uint32_t numOuterVerts = static_cast<uint32_t>(vertices_.size()) - baseIdx - 1;
    for (uint32_t i = 0; i < numOuterVerts; ++i) {
        uint32_t next = (i + 1) % numOuterVerts;
        addTriangleIndices(baseIdx, baseIdx + 1 + i, baseIdx + 1 + next);
    }

    addCommand(DrawCommandType::DrawTriangles, static_cast<uint32_t>(vertices_.size() - baseIdx), numOuterVerts * 3);
}

void DrawList::drawLine(const Vec2& p1, const Vec2& p2, Color color, float thickness) {
    Vec2 dir = p2 - p1;
    float len = dir.length();
    if (len < 0.0001f)
        return;

    dir = dir * (1.0f / len);
    Vec2 normal = dir.perpendicular() * (thickness * 0.5f);

    reserveVertices(4);
    reserveIndices(6);

    uint32_t baseIdx = static_cast<uint32_t>(vertices_.size());
    Vec2 uv(0.0f, 0.0f);

    vertices_.push_back(Vertex(p1 + normal, uv, color));
    vertices_.push_back(Vertex(p1 - normal, uv, color));
    vertices_.push_back(Vertex(p2 - normal, uv, color));
    vertices_.push_back(Vertex(p2 + normal, uv, color));

    addTriangleIndices(baseIdx + 0, baseIdx + 1, baseIdx + 2);
    addTriangleIndices(baseIdx + 0, baseIdx + 2, baseIdx + 3);

    addCommand(DrawCommandType::DrawTriangles, 4, 6);
}

void DrawList::drawCircle(const Vec2& center, float radius, Color color, int segments) {
    if (segments < 3)
        segments = 3;

    float angleStep = 2.0f * PI / segments;
    Vec2 prevPoint = center + Vec2(radius, 0.0f);

    for (int i = 1; i <= segments; ++i) {
        float angle = angleStep * i;
        Vec2 point = center + Vec2(std::cos(angle) * radius, std::sin(angle) * radius);
        drawLine(prevPoint, point, color, 1.0f);
        prevPoint = point;
    }
}

void DrawList::drawCircleFilled(const Vec2& center, float radius, Color color, int segments) {
    if (segments < 3)
        segments = 3;

    reserveVertices(segments + 1);
    reserveIndices(segments * 3);

    uint32_t baseIdx = static_cast<uint32_t>(vertices_.size());
    Vec2 uv(0.0f, 0.0f);

    // Center vertex
    vertices_.push_back(Vertex(center, uv, color));

    // Outer vertices
    float angleStep = 2.0f * PI / segments;
    for (int i = 0; i < segments; ++i) {
        float angle = angleStep * i;
        Vec2 pos = center + Vec2(std::cos(angle) * radius, std::sin(angle) * radius);
        vertices_.push_back(Vertex(pos, uv, color));
    }

    // Triangle fan
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        addTriangleIndices(baseIdx, baseIdx + 1 + i, baseIdx + 1 + next);
    }

    addCommand(DrawCommandType::DrawTriangles, segments + 1, segments * 3);
}

void DrawList::drawTriangle(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color color) {
    drawLine(p1, p2, color, 1.0f);
    drawLine(p2, p3, color, 1.0f);
    drawLine(p3, p1, color, 1.0f);
}

void DrawList::drawTriangleFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color color) {
    reserveVertices(3);
    reserveIndices(3);

    uint32_t baseIdx = static_cast<uint32_t>(vertices_.size());
    Vec2 uv(0.0f, 0.0f);

    vertices_.push_back(Vertex(p1, uv, color));
    vertices_.push_back(Vertex(p2, uv, color));
    vertices_.push_back(Vertex(p3, uv, color));

    addTriangleIndices(baseIdx, baseIdx + 1, baseIdx + 2);

    addCommand(DrawCommandType::DrawTriangles, 3, 3);
}

void DrawList::drawText(const Vec2& position, const char* text, Color color, float fontSize) {
    // Placeholder - will be implemented with text subsystem
    // For now, draw a placeholder rectangle
    float width = fontSize * 0.5f * std::strlen(text);
    drawRectFilled(Rect(position.x, position.y, width, fontSize), color.withAlpha(64));
}

void DrawList::pushClipRect(const Rect& rect) {
    clipRectStack_.push_back(currentClipRect_);
    currentClipRect_ = currentClipRect_.intersection(rect);

    DrawCommand cmd;
    cmd.type = DrawCommandType::SetClipRect;
    cmd.clipRect = currentClipRect_;
    commands_.push_back(cmd);
}

void DrawList::popClipRect() {
    if (!clipRectStack_.empty()) {
        currentClipRect_ = clipRectStack_.back();
        clipRectStack_.pop_back();

        DrawCommand cmd;
        cmd.type = DrawCommandType::SetClipRect;
        cmd.clipRect = currentClipRect_;
        commands_.push_back(cmd);
    }
}

void DrawList::setTexture(uint64_t textureID) {
    if (currentTexture_ != textureID) {
        currentTexture_ = textureID;

        DrawCommand cmd;
        cmd.type = DrawCommandType::SetTexture;
        cmd.textureID = textureID;
        commands_.push_back(cmd);
    }
}

} // namespace dakt::gui
