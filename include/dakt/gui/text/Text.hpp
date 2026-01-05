#ifndef DAKT_GUI_TEXT_HPP
#define DAKT_GUI_TEXT_HPP

#include "TextShaper.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace dakt::gui {

// Forward declarations
class Font;
class GlyphAtlas;

// ============================================================================
// Text Rendering Parameters
// ============================================================================

struct TextRenderParams {
    float fontSize = 12.0f;
    Color color = Color(255, 255, 255, 255);
    float opacity = 1.0f;
    bool antialiased = true;
    bool bold = false;
    bool italic = false;
    float letterSpacing = 0.0f;
    float lineHeight = 1.2f;
    uint32_t maxWidth = 0; // 0 = no wrapping
};

// ============================================================================
// Text Layout
// ============================================================================

struct TextLine {
    std::vector<ShapedGlyph> glyphs;
    float width = 0.0f;
    float height = 0.0f;
    uint32_t startCluster = 0;
    uint32_t endCluster = 0;
};

struct TextLayout {
    std::vector<TextLine> lines;
    float totalWidth = 0.0f;
    float totalHeight = 0.0f;
    uint32_t lineCount = 0;
};

// ============================================================================
// Text Renderer
// ============================================================================

class TextRenderer {
  public:
    TextRenderer();
    ~TextRenderer();

    // Load font
    bool loadFont(const std::string& name, const std::string& filePath);
    Font* getFont(const std::string& name);

    // Shape and layout text
    ShapedRun shapeText(const std::string& fontName, const std::string& text);
    TextLayout layoutText(const std::string& fontName, const std::string& text, const TextRenderParams& params);

    // Get glyph atlas for rendering
    GlyphAtlas& getAtlas(const std::string& fontName);

    // Text metrics
    Vec2 measureText(const std::string& fontName, const std::string& text, float fontSize);
    float measureLine(const std::string& fontName, const std::string& text, float fontSize);

  private:
    std::map<std::string, std::unique_ptr<Font>> fonts_;
    std::map<std::string, std::unique_ptr<GlyphAtlas>> atlases_;
    TextShaper shaper_;
};

} // namespace dakt::gui

#endif