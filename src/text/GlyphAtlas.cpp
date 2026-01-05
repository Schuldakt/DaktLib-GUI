#include "dakt/gui/text/GlyphAtlas.hpp"
#include "dakt/gui/text/Font.hpp"
#include <algorithm>
#include <cmath>

namespace dakt::gui::text {

GlyphAtlas::GlyphAtlas(uint32_t pageWidth, uint32_t pageHeight) : pageWidth_(pageWidth), pageHeight_(pageHeight) {
    // Create first page
    pages_.emplace_back();
    pages_[0].width = pageWidth_;
    pages_[0].height = pageHeight_;
    pages_[0].pixelData.resize(pageWidth_ * pageHeight_, 128);
}

GlyphAtlas::~GlyphAtlas() = default;

bool GlyphAtlas::addGlyph(Font& font, uint32_t glyphID, float fontSize) {
    // Stub implementation
    return false;
}

// Calculate glyph size in pixels
bool GlyphAtlas::hasGlyph(uint32_t glyphID, float fontSize) const {
    return false; // Stub
}

const AtlasGlyph& GlyphAtlas::getGlyph(uint32_t glyphID, float fontSize) const {
    static AtlasGlyph empty{};
    return empty;
}

void GlyphAtlas::clear() { glyphMap_.clear(); }

void GlyphAtlas::regenerate(Font& font, float fontSize) {}

bool GlyphAtlas::saveToFile(const std::string& filePath) const { return false; }

bool GlyphAtlas::loadFromFile(const std::string& filePath) { return false; }

bool GlyphAtlas::packGlyph(uint32_t width, uint32_t height, uint32_t& outX, uint32_t& outY) {
    outX = 0;
    outY = 0;
    return true;
}

} // namespace dakt::gui::text
