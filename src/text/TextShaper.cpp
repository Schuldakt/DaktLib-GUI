#include "dakt/gui/text/TextShaper.hpp"
#include "dakt/gui/text/Font.hpp"
#include <algorithm>

namespace dakt::gui::text {

TextShaper::TextShaper() = default;
TextShaper::~TextShaper() = default;

ShapedRun TextShaper::shape(Font& font, const std::string& text, uint32_t scriptTag) {
    ShapedRun run;
    run.scriptTag = scriptTag;

    // Convert UTF-8 text to codepoints and get glyphs
    uint32_t cluster = 0;
    for (size_t i = 0; i < text.length(); ++i) {
        unsigned char c = text[i];
        uint32_t codepoint = 0;

        // Simplified UTF-8 decoding (assumes mostly ASCII)
        if ((c & 0x80) == 0) {
            codepoint = c;
        } else {
            // Skip multi-byte sequences for now
            continue;
        }

        uint16_t glyphID = font.getGlyphId(codepoint);
        const Glyph* glyph = font.getGlyph(glyphID);
        if (!glyph)
            continue;

        ShapedGlyph shapedGlyph;
        shapedGlyph.glyphID = glyphID;
        shapedGlyph.cluster = cluster;
        shapedGlyph.xAdvance = glyph->advanceWidth;
        shapedGlyph.yAdvance = 0;
        shapedGlyph.xOffset = 0;
        shapedGlyph.yOffset = 0;

        run.glyphs.push_back(shapedGlyph);
        ++cluster;
    }

    // Apply GSUB and GPOS
    applyGSUB(font, run);
    applyGPOS(font, run);

    return run;
}

std::vector<ShapedRun> TextShaper::shapeBidi(Font& font, const std::string& text) {
    std::vector<ShapedRun> runs;

    // TODO: Implement bidirectional text algorithm
    // For now, just shape as single run
    runs.push_back(shape(font, text));

    return runs;
}

void TextShaper::applyGSUB(Font& font, ShapedRun& run) {
    // TODO: Apply GSUB (substitution) table
    // - Ligature substitution (ff -> ffi, etc.)
    // - Contextual substitution
    // - Alternate glyph selection
}

void TextShaper::applyGPOS(Font& font, ShapedRun& run) {
    // TODO: Apply GPOS (positioning) table
    // - Kerning adjustments
    // - Mark positioning
    // - Cursive attachment
}

void TextShaper::substituteWhitespace(ShapedRun& run, uint32_t spaceGlyphID) {
    // TODO: Replace space characters with proper space glyph
    for (auto& glyph : run.glyphs) {
        // Check if glyph is space and substitute
    }
}

} // namespace dakt::gui::text