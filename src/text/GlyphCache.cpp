#include "dakt/gui/text/GlyphCache.hpp"
#include "dakt/gui/text/Font.hpp"
#include "dakt/gui/text/GlyphAtlas.hpp"
#include <algorithm>

namespace dakt::gui {

GlyphCache::GlyphCache(size_t maxEntries) : maxEntries_(maxEntries) {}

GlyphCache::~GlyphCache() = default;

const CachedTextRun* GlyphCache::get(Font& font, float fontSize, const std::string& text, GlyphAtlas& atlas) {
    TextCacheKey key{&font, fontSize, text, 0};

    auto it = cache_.find(key);
    if (it != cache_.end()) {
        // Cache hit - move to front of LRU
        ++hitCount_;
        lruList_.erase(it->second.lruIterator);
        lruList_.push_front(key);
        it->second.lruIterator = lruList_.begin();
        it->second.run.lastAccessFrame = currentFrame_;
        return &it->second.run;
    }

    // Cache miss - create new entry
    ++missCount_;

    // Evict if at capacity
    if (cache_.size() >= maxEntries_) {
        evictLRU();
    }

    // Create and cache the entry
    CacheEntry entry;
    entry.run = createEntry(font, fontSize, text, atlas);
    entry.run.lastAccessFrame = currentFrame_;

    lruList_.push_front(key);
    entry.lruIterator = lruList_.begin();

    auto [insertIt, success] = cache_.emplace(key, std::move(entry));
    return &insertIt->second.run;
}

bool GlyphCache::has(Font& font, float fontSize, const std::string& text) const {
    TextCacheKey key{&font, fontSize, text, 0};
    return cache_.find(key) != cache_.end();
}

void GlyphCache::invalidate(Font& font, float fontSize, const std::string& text) {
    TextCacheKey key{&font, fontSize, text, 0};
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        lruList_.erase(it->second.lruIterator);
        cache_.erase(it);
    }
}

void GlyphCache::invalidateFont(const Font* font) {
    auto it = cache_.begin();
    while (it != cache_.end()) {
        if (it->first.font == font) {
            lruList_.erase(it->second.lruIterator);
            it = cache_.erase(it);
        } else {
            ++it;
        }
    }
}

void GlyphCache::clear() {
    cache_.clear();
    lruList_.clear();
}

void GlyphCache::newFrame() { ++currentFrame_; }

void GlyphCache::evictLRU() {
    if (lruList_.empty())
        return;

    // Remove least recently used (back of list)
    const auto& lruKey = lruList_.back();
    cache_.erase(lruKey);
    lruList_.pop_back();
}

CachedTextRun GlyphCache::createEntry(Font& font, float fontSize, const std::string& text, GlyphAtlas& atlas) {
    CachedTextRun run;
    run.totalWidth = 0;
    run.ascender = font.getAscender() * fontSize / font.getUnitsPerEm();
    run.descender = font.getDescender() * fontSize / font.getUnitsPerEm();
    run.totalHeight = run.ascender - run.descender;

    float cursorX = 0;
    float cursorY = 0;

    // Simple UTF-8 decoding and glyph positioning
    for (size_t i = 0; i < text.length();) {
        uint32_t codepoint = 0;
        unsigned char c = text[i];

        // UTF-8 decode
        if ((c & 0x80) == 0) {
            // ASCII
            codepoint = c;
            ++i;
        } else if ((c & 0xE0) == 0xC0) {
            // 2-byte sequence
            codepoint = (c & 0x1F) << 6;
            if (i + 1 < text.length()) {
                codepoint |= (text[i + 1] & 0x3F);
            }
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 3-byte sequence
            codepoint = (c & 0x0F) << 12;
            if (i + 1 < text.length()) {
                codepoint |= (text[i + 1] & 0x3F) << 6;
            }
            if (i + 2 < text.length()) {
                codepoint |= (text[i + 2] & 0x3F);
            }
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // 4-byte sequence
            codepoint = (c & 0x07) << 18;
            if (i + 1 < text.length()) {
                codepoint |= (text[i + 1] & 0x3F) << 12;
            }
            if (i + 2 < text.length()) {
                codepoint |= (text[i + 2] & 0x3F) << 6;
            }
            if (i + 3 < text.length()) {
                codepoint |= (text[i + 3] & 0x3F);
            }
            i += 4;
        } else {
            // Invalid UTF-8
            ++i;
            continue;
        }

        // Get glyph ID
        uint16_t glyphID = font.getGlyphId(codepoint);
        if (glyphID == 0 && codepoint != 0) {
            // Use .notdef glyph (ID 0)
            glyphID = 0;
        }

        // Ensure glyph is in atlas
        if (!atlas.hasGlyph(glyphID, fontSize)) {
            atlas.addGlyph(font, glyphID, fontSize);
        }

        // Get glyph from atlas
        const AtlasGlyph& atlasGlyph = atlas.getGlyph(glyphID, fontSize);

        // Position the glyph
        CachedGlyphPosition pos;
        pos.glyphID = glyphID;
        pos.x = cursorX + atlasGlyph.bearingX;
        pos.y = cursorY + run.ascender - atlasGlyph.bearingY;
        pos.width = atlasGlyph.width;
        pos.height = atlasGlyph.height;
        pos.atlasX = atlasGlyph.atlasX;
        pos.atlasY = atlasGlyph.atlasY;
        pos.atlasW = atlasGlyph.atlasWidth;
        pos.atlasH = atlasGlyph.atlasHeight;
        pos.atlasPage = atlasGlyph.pageIndex;

        run.glyphs.push_back(pos);

        cursorX += atlasGlyph.advanceWidth;
    }

    run.totalWidth = cursorX;

    return run;
}

} // namespace dakt::gui
