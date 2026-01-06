#ifndef DAKTLIB_GUI_GLYPH_CACHE_HPP
#define DAKTLIB_GUI_GLYPH_CACHE_HPP

#include "../core/Types.hpp"
#include <cstdint>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

namespace dakt::gui {

// Forward declarations
class Font;
class GlyphAtlas;

/**
 * @brief Key for caching shaped text runs
 */
struct TextCacheKey {
    const Font* font;
    float fontSize;
    std::string text;
    uint32_t variationHash; // Hash of variation axis values

    bool operator==(const TextCacheKey& other) const { return font == other.font && fontSize == other.fontSize && text == other.text && variationHash == other.variationHash; }
};

struct TextCacheKeyHash {
    size_t operator()(const TextCacheKey& key) const {
        size_t h = std::hash<const void*>{}(key.font);
        h ^= std::hash<float>{}(key.fontSize) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<std::string>{}(key.text) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<uint32_t>{}(key.variationHash) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

/**
 * @brief Cached glyph positioning info for a text run
 */
struct CachedGlyphPosition {
    uint32_t glyphID;
    float x, y; // Position relative to text origin
    float width, height;
    float atlasX, atlasY, atlasW, atlasH; // UV coordinates in atlas
    uint32_t atlasPage;
};

/**
 * @brief Cached text run with positioned glyphs
 */
struct CachedTextRun {
    std::vector<CachedGlyphPosition> glyphs;
    float totalWidth;
    float totalHeight;
    float ascender;
    float descender;
    uint64_t lastAccessFrame;
};

/**
 * @brief LRU cache for shaped and positioned text runs
 *
 * Caches the results of text shaping and glyph lookup to avoid
 * repeated expensive operations for static text.
 */
class GlyphCache {
  public:
    explicit GlyphCache(size_t maxEntries = 1024);
    ~GlyphCache();

    /**
     * Get or create a cached text run
     * @param font Font to use
     * @param fontSize Font size in pixels
     * @param text UTF-8 text string
     * @param atlas Glyph atlas to lookup/add glyphs
     * @return Cached text run with positioned glyphs
     */
    const CachedTextRun* get(Font& font, float fontSize, const std::string& text, GlyphAtlas& atlas);

    /**
     * Check if text run is already cached
     */
    bool has(Font& font, float fontSize, const std::string& text) const;

    /**
     * Invalidate a specific cache entry
     */
    void invalidate(Font& font, float fontSize, const std::string& text);

    /**
     * Invalidate all entries for a font
     */
    void invalidateFont(const Font* font);

    /**
     * Clear entire cache
     */
    void clear();

    /**
     * Called each frame to update access tracking
     */
    void newFrame();

    /**
     * Set maximum cache entries
     */
    void setMaxEntries(size_t maxEntries) { maxEntries_ = maxEntries; }
    size_t getMaxEntries() const { return maxEntries_; }

    /**
     * Get current cache statistics
     */
    size_t getEntryCount() const { return cache_.size(); }
    size_t getHitCount() const { return hitCount_; }
    size_t getMissCount() const { return missCount_; }
    void resetStats() {
        hitCount_ = 0;
        missCount_ = 0;
    }

  private:
    // Create new cache entry
    CachedTextRun createEntry(Font& font, float fontSize, const std::string& text, GlyphAtlas& atlas);

    // Evict least recently used entries
    void evictLRU();

    // LRU list (front = most recent, back = least recent)
    using LRUList = std::list<TextCacheKey>;
    LRUList lruList_;

    // Cache map
    struct CacheEntry {
        CachedTextRun run;
        LRUList::iterator lruIterator;
    };
    std::unordered_map<TextCacheKey, CacheEntry, TextCacheKeyHash> cache_;

    size_t maxEntries_;
    uint64_t currentFrame_ = 0;
    size_t hitCount_ = 0;
    size_t missCount_ = 0;
};

} // namespace dakt::gui

#endif
