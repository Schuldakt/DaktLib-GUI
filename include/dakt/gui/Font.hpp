// ============================================================================
// DaktLib GUI Module - Font System
// ============================================================================
// TrueType/OpenType font loading and glyph rendering.
// ============================================================================

#pragma once

#include <dakt/core/Buffer.hpp>
#include <dakt/gui/Types.hpp>

#include <memory>
#include <span>
#include <unordered_map>
#include <vector>

namespace dakt::gui
{

// ============================================================================
// Glyph Information
// ============================================================================

struct GlyphMetrics
{
    f32 advance = 0.0f;      // Horizontal advance
    f32 leftBearing = 0.0f;  // Left side bearing
    f32 width = 0.0f;        // Glyph width
    f32 height = 0.0f;       // Glyph height
    f32 offsetX = 0.0f;      // X offset from cursor
    f32 offsetY = 0.0f;      // Y offset from baseline
};

struct GlyphInfo
{
    u32 codepoint = 0;
    GlyphMetrics metrics;
    Vec2 uvMin;         // Top-left UV in atlas
    Vec2 uvMax;         // Bottom-right UV in atlas
    u32 atlasPage = 0;  // Which atlas texture page
};

// ============================================================================
// Font Atlas
// ============================================================================

class FontAtlas
{
public:
    FontAtlas();
    ~FontAtlas();

    // Atlas building
    void clear();
    bool build();

    // Access atlas texture data
    [[nodiscard]] std::span<const u8> pixels() const { return m_pixels; }
    [[nodiscard]] i32 width() const { return m_width; }
    [[nodiscard]] i32 height() const { return m_height; }
    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void setDirty(bool dirty = true) { m_dirty = dirty; }
    void clearDirty() { m_dirty = false; }

    // Texture ID (set by backend after upload)
    void setTextureId(uintptr_t id) { m_textureId = id; }
    [[nodiscard]] uintptr_t textureId() const { return m_textureId; }

    // Custom rectangles (for icons, etc.)
    struct CustomRect
    {
        u32 id = 0;
        i32 width = 0;
        i32 height = 0;
        i32 x = 0;     // Set after packing
        i32 y = 0;     // Set after packing
        u32 page = 0;  // Atlas page
    };

    u32 addCustomRect(i32 width, i32 height);
    [[nodiscard]] const CustomRect* getCustomRect(u32 id) const;

private:
    friend class Font;

    std::vector<u8> m_pixels;
    i32 m_width = 0;
    i32 m_height = 0;
    uintptr_t m_textureId = 0;
    bool m_dirty = true;

    std::vector<CustomRect> m_customRects;
    u32 m_nextCustomRectId = 1;
};

// ============================================================================
// Font Configuration
// ============================================================================

struct FontConfig
{
    f32 sizePixels = 14.0f;   // Font size in pixels
    i32 oversampleH = 3;      // Horizontal oversampling
    i32 oversampleV = 1;      // Vertical oversampling
    bool pixelSnapH = false;  // Snap to pixel horizontally

    // Unicode ranges to include
    std::vector<std::pair<u32, u32>> glyphRanges;

    // Offset and spacing adjustments
    Vec2 glyphOffset = {0, 0};
    f32 glyphExtraSpacing = 0.0f;

    // Rasterizer settings
    bool antiAlias = true;
    u8 padding = 1;

    // Get default glyph ranges
    static std::vector<std::pair<u32, u32>> rangesDefault();
    static std::vector<std::pair<u32, u32>> rangesLatin();
    static std::vector<std::pair<u32, u32>> rangesKorean();
    static std::vector<std::pair<u32, u32>> rangesChinese();
    static std::vector<std::pair<u32, u32>> rangesJapanese();
    static std::vector<std::pair<u32, u32>> rangesCyrillic();
    static std::vector<std::pair<u32, u32>> rangesThai();
    static std::vector<std::pair<u32, u32>> rangesVietnamese();
};

// ============================================================================
// Font
// ============================================================================

class Font
{
public:
    Font();
    ~Font();

    // Loading
    bool loadFromFile(StringView path, const FontConfig& config = {});
    bool loadFromMemory(std::span<const u8> data, const FontConfig& config = {});
    bool loadDefault(f32 sizePixels = 14.0f);

    // Properties
    [[nodiscard]] f32 fontSize() const { return m_fontSize; }
    [[nodiscard]] f32 ascent() const { return m_ascent; }
    [[nodiscard]] f32 descent() const { return m_descent; }
    [[nodiscard]] f32 lineHeight() const { return m_lineHeight; }
    [[nodiscard]] bool isLoaded() const { return m_loaded; }

    // Glyph access
    [[nodiscard]] const GlyphInfo* findGlyph(u32 codepoint) const;
    [[nodiscard]] const GlyphInfo& getFallbackGlyph() const { return m_fallbackGlyph; }

    // Kerning
    [[nodiscard]] f32 getKerning(u32 left, u32 right) const;

    // Text measurement
    [[nodiscard]] Vec2 calcTextSize(StringView text, f32 wrapWidth = 0.0f) const;
    [[nodiscard]] f32 calcTextWidth(StringView text) const;
    [[nodiscard]] usize calcTextWrapPosition(StringView text, f32 wrapWidth) const;

    // Rendering helpers
    void renderText(DrawList& drawList, Vec2 pos, StringView text, Color color, f32 clipMaxX = 0.0f) const;

    // Atlas access
    [[nodiscard]] FontAtlas* atlas() const { return m_atlas; }
    void setAtlas(FontAtlas* atlas) { m_atlas = atlas; }

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    FontAtlas* m_atlas = nullptr;

    f32 m_fontSize = 14.0f;
    f32 m_ascent = 0.0f;
    f32 m_descent = 0.0f;
    f32 m_lineHeight = 0.0f;
    f32 m_scale = 1.0f;
    bool m_loaded = false;

    std::unordered_map<u32, GlyphInfo> m_glyphs;
    std::unordered_map<u64, f32> m_kerningPairs;
    GlyphInfo m_fallbackGlyph;

    bool buildGlyphs(const FontConfig& config);
};

// ============================================================================
// Font Manager
// ============================================================================

class FontManager
{
public:
    FontManager();
    ~FontManager();

    // Font loading
    Font* addFont(StringView path, f32 sizePixels, const FontConfig& config = {});
    Font* addFontDefault(f32 sizePixels = 14.0f);
    Font* addFontFromMemory(std::span<const u8> data, f32 sizePixels, const FontConfig& config = {});

    // Merge fonts (for adding icon fonts)
    bool mergeFonts(Font* target, Font* source);

    // Access fonts
    [[nodiscard]] Font* getFont(usize index) const;
    [[nodiscard]] Font* getDefaultFont() const { return m_defaultFont; }
    [[nodiscard]] usize fontCount() const { return m_fonts.size(); }

    // Atlas management
    [[nodiscard]] FontAtlas* atlas() { return &m_atlas; }
    bool buildAtlas();

    // Clear all fonts
    void clear();

private:
    std::vector<std::unique_ptr<Font>> m_fonts;
    Font* m_defaultFont = nullptr;
    FontAtlas m_atlas;
};

}  // namespace dakt::gui
