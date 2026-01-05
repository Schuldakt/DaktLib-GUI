#ifndef DAKT_GUI_GLYPH_ATLAS_HPP
#define DAKT_GUI_GLYPH_ATLAS_HPP

#include "../core/Types.hpp"
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace dakt::gui {

// Forward declarations
class Font;

// ============================================================================
// SDF Glyph Atlas
// ============================================================================

struct AtlasGlyph {
    uint32_t glyphID;
    float fontSize;

    // Position in atlas texture (normalized 0-1)
    float atlasX, atlasY;
    float atlasWidth, atlasHeight;

    // Metrics for rendering
    float advanceWidth;
    float bearingX, bearingY; // Offsets from origin
    float width, height;      // Glyph dimensions in pixels

    uint32_t pageIndex = 0; // Which atlas page (if multiple)
};

struct AtlasPage {
    uint32_t width = 512;
    uint32_t height = 512;
    std::vector<uint8_t> pixelData; // Grayscale SDF data
    float pixelsPerEmUnit = 1.0f;   // Scale from font units to pixels
};

class GlyphAtlas {
  public:
    GlyphAtlas(uint32_t pageWidth = 512, uint32_t pageHeight = 512);
    ~GlyphAtlas();

    // Add glyph to atlas
    bool addGlyph(Font& font, uint32_t glyphID, float fontSize);
    bool hasGlyph(uint32_t glyphID, float fontSize) const;
    const AtlasGlyph& getGlyph(uint32_t glyphID, float fontSize) const;

    // Atlas pages
    uint32_t getPageCount() const { return pages_.size(); }
    const AtlasPage& getPage(uint32_t pageIndex) const { return pages_[pageIndex]; }

    // Rasterization parameters
    void setSDFSpread(uint32_t spread) { sdfSpread_ = spread; }
    void setMSDF(bool enabled) { enableMSDF_ = enabled; }

    // Clear and rebuild
    void clear();
    void regenerate(Font& font, float fontSize);

    // Load/save atlas format
    bool saveToFile(const std::string& filePath) const;
    bool loadFromFile(const std::string& filePath);

  private:
    // Packing
    struct PackRect {
        uint32_t x, y, w, h;
        bool used = false;
    };

    bool packGlyph(uint32_t width, uint32_t height, uint32_t& outX, uint32_t& outY);

    // TODO: Rendering methods - declared in cpp for now
    // void rasterizeGlyph(const Glyph& glyph, uint8_t* sdfData, uint32_t width, uint32_t height, float spread);
    // void rasterizeGlyphMSDF(const Glyph& glyph, uint8_t* msdfData, uint32_t width, uint32_t height, float spread);

    uint32_t pageWidth_, pageHeight_;
    std::vector<AtlasPage> pages_;
    std::vector<PackRect> packRects_; // Current page packing state

    std::map<std::pair<uint32_t, uint32_t>, AtlasGlyph> glyphMap_; // (glyphID, fontSize hash) -> AtlasGlyph

    uint32_t sdfSpread_ = 2;
    bool enableMSDF_ = false;
};

} // namespace dakt::gui

#endif
