#include "dakt/gui/text/Text.hpp"
#include "dakt/gui/text/Font.hpp"
#include "dakt/gui/text/GlyphAtlas.hpp"
#include <algorithm>

namespace dakt::gui::text {

TextRenderer::TextRenderer() = default;
TextRenderer::~TextRenderer() = default;

bool TextRenderer::loadFont(const std::string& name, const std::string& filePath) {
    auto font = std::make_unique<Font>();
    if (!font->loadFromFile(filePath)) {
        return false;
    }
    fonts_[name] = std::move(font);
    atlases_[name] = std::make_unique<GlyphAtlas>();
    return true;
}

Font* TextRenderer::getFont(const std::string& name) {
    auto it = fonts_.find(name);
    if (it != fonts_.end()) {
        return it->second.get();
    }
    return nullptr;
}

ShapedRun TextRenderer::shapeText(const std::string& fontName, const std::string& text) {
    Font* font = getFont(fontName);
    if (!font) {
        return ShapedRun();
    }
    return shaper_.shape(*font, text);
}

TextLayout TextRenderer::layoutText(const std::string& fontName, const std::string& text, const TextRenderParams& params) {
    TextLayout layout;

    Font* font = getFont(fontName);
    if (!font) {
        return layout;
    }

    // Shape text
    ShapedRun run = shaper_.shape(*font, text);

    // Layout shaped glyphs into lines
    TextLine currentLine;
    float currentLineWidth = 0.0f;

    for (const auto& glyph : run.glyphs) {
        const Glyph* fontGlyph = font->getGlyph(glyph.glyphID);
        float advanceWidth = (fontGlyph) ? font->pixelsFromUnits(fontGlyph->advanceWidth, params.fontSize) : 0;

        // Check if we need to wrap
        if (params.maxWidth > 0 && currentLineWidth + advanceWidth > params.maxWidth) {
            layout.lines.push_back(currentLine);
            currentLine.glyphs.clear();
            currentLineWidth = 0.0f;
        }

        currentLine.glyphs.push_back(glyph);
        currentLineWidth += advanceWidth;
    }

    if (!currentLine.glyphs.empty()) {
        layout.lines.push_back(currentLine);
    }

    // Calculate layout dimensions
    layout.lineCount = layout.lines.size();
    layout.totalHeight = layout.lineCount * params.lineHeight * params.fontSize;
    layout.totalWidth = params.maxWidth > 0 ? params.maxWidth : currentLineWidth;

    return layout;
}

GlyphAtlas& TextRenderer::getAtlas(const std::string& fontName) { return *atlases_[fontName]; }

Vec2 TextRenderer::measureText(const std::string& fontName, const std::string& text, float fontSize) {
    Font* font = getFont(fontName);
    if (!font) {
        return Vec2(0, 0);
    }

    ShapedRun run = shaper_.shape(*font, text);
    float width = 0.0f;

    for (const auto& glyph : run.glyphs) {
        const Glyph* fontGlyph = font->getGlyph(glyph.glyphID);
        if (fontGlyph) {
            width += font->pixelsFromUnits(fontGlyph->advanceWidth, fontSize);
        }
    }

    float height = font->pixelsFromUnits(font->getAscender() - font->getDescender(), fontSize);

    return Vec2(width, height);
}

float TextRenderer::measureLine(const std::string& fontName, const std::string& text, float fontSize) {
    Font* font = getFont(fontName);
    if (!font) {
        return 0.0f;
    }

    float width = 0.0f;
    for (char c : text) {
        uint32_t codepoint = static_cast<unsigned char>(c);
        uint16_t glyphID = font->getGlyphId(codepoint);
        const Glyph* glyph = font->getGlyph(glyphID);
        if (glyph) {
            width += font->pixelsFromUnits(glyph->advanceWidth, fontSize);
        }
    }

    return width;
}

} // namespace dakt::gui::text
