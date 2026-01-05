#include "dakt/gui/text/Font.hpp"
#include "dakt/gui/text/TTFParser.hpp"

namespace dakt::gui {

Font::Font() = default;

Font::~Font() = default;

bool Font::loadFromFile(const std::string& filePath) {
    // Stub implementation
    familyName_ = "Arial";
    fullName_ = "Arial Regular";
    return false;
}

bool Font::loadFromMemory(const uint8_t* data, size_t size) {
    // Stub implementation
    familyName_ = "Arial";
    fullName_ = "Arial Regular";
    return false;
}

uint16_t Font::getGlyphId(uint32_t codepoint) const {
    // Stub: return simple mapping
    return (codepoint >= 32 && codepoint < 127) ? (codepoint - 32) : 0;
}

const Glyph* Font::getGlyph(uint16_t glyphId) const {
    auto& glyphCache = const_cast<std::unordered_map<uint16_t, Glyph>&>(glyphCache_);
    auto it = glyphCache.find(glyphId);
    if (it != glyphCache.end()) {
        return &it->second;
    }

    Glyph glyph;
    glyph.glyphID = glyphId;
    glyph.advanceWidth = 500;
    glyph.leftSideBearing = 50;
    glyph.xMin = 0;
    glyph.yMin = -200;
    glyph.xMax = 450;
    glyph.yMax = 800;

    auto& cached = glyphCache[glyphId];
    cached = glyph;
    return &cached;
}

float Font::pixelsFromUnits(float units, float fontSize) const {
    if (unitsPerEm_ <= 0)
        return 0;
    return (units / unitsPerEm_) * fontSize;
}

float Font::unitsFromPixels(float pixels, float fontSize) const {
    if (fontSize <= 0)
        return 0;
    return (pixels / fontSize) * unitsPerEm_;
}

} // namespace dakt::gui