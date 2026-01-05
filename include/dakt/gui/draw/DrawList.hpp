#ifndef DAKT_GUI_DRAW_HPP
#define DAKT_GUI_DRAW_HPP

#include "../core/Types.hpp"
#include <vector>

namespace dakt::gui {

// ============================================================================
// Vertex Structure
// ============================================================================

struct DAKT_GUI_API Vertex {
    Vec2 position;
    Vec2 uv;
    Color color;

    Vertex() = default;
    Vertex(const Vec2& pos, const Vec2& texCoord, const Color& col) : position(pos), uv(texCoord), color(col) {}
};

// ============================================================================
// Draw Command
// ============================================================================

enum class DrawCommandType { None, DrawTriangles, SetClipRect, SetTexture };

struct DAKT_GUI_API DrawCommand {
    DrawCommandType type = DrawCommandType::None;
    uint32_t vertexOffset = 0;
    uint32_t vertexCount = 0;
    uint32_t indexOffset = 0;
    uint32_t indexCount = 0;
    Rect clipRect;
    uint64_t textureID = 0;
};

// ============================================================================
// Draw List
// ============================================================================

class DAKT_GUI_API DrawList {
  public:
    DrawList();
    ~DrawList();

    void reset();

    // Primitive drawing
    void drawRect(const Rect& rect, Color color);
    void drawRectFilled(const Rect& rect, Color color);
    void drawRectRounded(const Rect& rect, Color color, float radius);
    void drawRectFilledRounded(const Rect& rect, Color color, float radius);
    void drawRectFilledRounded(const Rect& rect, Color color, const BorderRadius& radius);

    void drawLine(const Vec2& p1, const Vec2& p2, Color color, float thickness = 1.0f);
    void drawCircle(const Vec2& center, float radius, Color color, int segments = 32);
    void drawCircleFilled(const Vec2& center, float radius, Color color, int segments = 32);

    void drawTriangle(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color color);
    void drawTriangleFilled(const Vec2& p1, const Vec2& p2, const Vec2& p3, Color color);

    // Text (placeholder - will use text subsystem)
    void drawText(const Vec2& position, const char* text, Color color, float fontSize = 14.0f);

    // Clipping
    void pushClipRect(const Rect& rect);
    void popClipRect();

    // Texture binding
    void setTexture(uint64_t textureID);

    // Direct vertex/index access
    void addVertex(const Vertex& vertex);
    void addIndex(uint32_t index);
    void addTriangleIndices(uint32_t i0, uint32_t i1, uint32_t i2);

    // Command buffer access
    const std::vector<Vertex>& getVertices() const { return vertices_; }
    const std::vector<uint32_t>& getIndices() const { return indices_; }
    const std::vector<DrawCommand>& getCommands() const { return commands_; }

    uint32_t getVertexCount() const { return static_cast<uint32_t>(vertices_.size()); }
    uint32_t getIndexCount() const { return static_cast<uint32_t>(indices_.size()); }

  private:
    std::vector<Vertex> vertices_;
    std::vector<uint32_t> indices_;
    std::vector<DrawCommand> commands_;
    std::vector<Rect> clipRectStack_;
    Rect currentClipRect_;
    uint64_t currentTexture_ = 0;

    void addCommand(DrawCommandType type, uint32_t vertexCount, uint32_t indexCount);
    void reserveVertices(size_t count);
    void reserveIndices(size_t count);

    // Helper for rounded corners
    void addArcVertices(const Vec2& center, float radius, float startAngle, float endAngle, Color color, int segments);
};

} // namespace dakt::gui

#endif
