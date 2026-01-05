#ifndef DAKT_GUI_TEXT_SHAPER_HPP
#define DAKT_GUI_TEXT_SHAPER_HPP

#include "../core/Types.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace dakt::gui {

// Forward declaration
class Font;

// ============================================================================
// Text Shaping
// ============================================================================

struct ShapedGlyph {
    uint32_t glyphID;
    uint32_t cluster; // Cluster index (maps back to input codepoint)
    float xAdvance;   // Advance width after glyph
    float yAdvance;   // Vertical advance (for vertical scripts)
    float xOffset;    // Offset from cluster origin
    float yOffset;
    uint32_t flags = 0; // Rendering flags
};

struct ShapedRun {
    std::vector<ShapedGlyph> glyphs;
    uint32_t scriptTag = 0;   // OpenType script tag (e.g., 'LATN')
    uint32_t languageTag = 0; // OpenType language tag
    bool isRTL = false;       // Right-to-left text
};

class TextShaper {
  public:
    TextShaper();
    ~TextShaper();

    // Shape text with font
    ShapedRun shape(Font& font, const std::string& text, uint32_t scriptTag = 0);

    // Shape bidirectional text
    std::vector<ShapedRun> shapeBidi(Font& font, const std::string& text);

    // Apply GSUB (substitution) features
    void applyGSUB(Font& font, ShapedRun& run);

    // Apply GPOS (positioning) features
    void applyGPOS(Font& font, ShapedRun& run);

    // Ligature substitution
    void substituteWhitespace(ShapedRun& run, uint32_t spaceGlyphID);

  private:
    // Bidirectional algorithm
    void determineBidiLevel(const std::string& text, std::vector<uint32_t>& levels);
    void reorderLogicalToVisual(std::vector<ShapedRun>& runs, const std::vector<uint32_t>& levels);

    // Feature application
    struct Feature {
        uint32_t tag;
        bool enabled;
    };

    std::vector<Feature> features_;
};

} // namespace dakt::gui

#endif
