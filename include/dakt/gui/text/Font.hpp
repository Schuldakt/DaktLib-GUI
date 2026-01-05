#ifndef DAKT_GUI_FONT_HPP
#define DAKT_GUI_FONT_HPP

#include "../core/Types.hpp"
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace dakt::gui {

// Forward declaration
class TTFParser;

// ============================================================================
// Font Structures
// ============================================================================

struct Glyph {
    uint32_t glyphID = 0;
    int16_t advanceWidth = 0;
    int16_t leftSideBearing = 0;
    int16_t xMin = 0, yMin = 0, xMax = 0, yMax = 0;
    // Contour data (simplified for now, full outline in parser)
};

// ============================================================================
// Font Class
// ============================================================================

class Font {
  public:
    Font();
    ~Font();

    // Load font from file
    bool loadFromFile(const std::string& filePath);
    bool loadFromMemory(const uint8_t* data, size_t size);

    // Font properties
    const std::string& getFamilyName() const { return familyName_; }
    const std::string& getFullName() const { return fullName_; }

    // Font metrics
    int16_t getUnitsPerEm() const { return unitsPerEm_; }
    int16_t getAscender() const { return ascender_; }
    int16_t getDescender() const { return descender_; }
    int16_t getLineGap() const { return lineGap_; }

    // Glyph access
    uint16_t getGlyphId(uint32_t codepoint) const;
    const Glyph* getGlyph(uint16_t glyphId) const;

    // Metrics conversion
    float pixelsFromUnits(float units, float fontSize) const;
    float unitsFromPixels(float pixels, float fontSize) const;

  private:
    std::unique_ptr<TTFParser> parser_;
    std::string filePath_;
    std::string familyName_;
    std::string fullName_;

    int16_t unitsPerEm_ = 1000;
    int16_t ascender_ = 800;
    int16_t descender_ = -200;
    int16_t lineGap_ = 0;

    mutable std::unordered_map<uint16_t, Glyph> glyphCache_;
};

} // namespace dakt::gui

#endif