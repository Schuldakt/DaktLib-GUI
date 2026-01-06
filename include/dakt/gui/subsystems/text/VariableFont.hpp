#ifndef DAKTLIB_GUI_VARIABLE_FONT_HPP
#define DAKTLIB_GUI_VARIABLE_FONT_HPP

#include "../core/Types.hpp"
#include "TTFParser.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace dakt::gui {

/**
 * @brief Variable font axis definition from FVAR table
 */
struct FontAxis {
    uint32_t tag;       // 4-character tag (e.g., 'wght', 'wdth', 'slnt')
    float minValue;     // Minimum axis value
    float defaultValue; // Default axis value
    float maxValue;     // Maximum axis value
    uint16_t flags;     // Axis flags
    uint16_t nameID;    // Name table ID for axis name
    std::string name;   // Human-readable name
};

/**
 * @brief Named instance (preset) from FVAR table
 */
struct FontInstance {
    uint16_t nameID;
    std::string name;
    std::vector<float> coordinates; // One value per axis
};

/**
 * @brief Axis value mapping from AVAR table
 */
struct AxisValueMap {
    float fromValue;
    float toValue;
};

/**
 * @brief Per-glyph variation deltas from GVAR table
 */
struct GlyphVariation {
    uint16_t glyphID;
    std::vector<std::pair<int16_t, int16_t>> deltas; // (dx, dy) per point
    std::vector<float> tupleCoords;                  // Which axes affect this variation
};

/**
 * @brief Variable font support - FVAR, GVAR, AVAR tables
 *
 * Enables smooth interpolation across design axes like
 * weight (100-900), width (50-200), slant, etc.
 */
class VariableFont {
  public:
    VariableFont();
    ~VariableFont();

    /**
     * Load variable font tables from TTFParser
     * @param parser TTFParser with loaded font data
     * @return true if variable font tables were found and parsed
     */
    bool load(TTFParser& parser);

    /**
     * Check if this font is a variable font
     */
    bool isVariable() const { return !axes_.empty(); }

    /**
     * Get all variation axes
     */
    const std::vector<FontAxis>& getAxes() const { return axes_; }

    /**
     * Get axis by tag (e.g., 'wght')
     */
    const FontAxis* getAxis(uint32_t tag) const;
    const FontAxis* getAxis(const char* tagStr) const;

    /**
     * Get named instances (presets)
     */
    const std::vector<FontInstance>& getInstances() const { return instances_; }

    /**
     * Set axis value (clamped to axis min/max)
     */
    void setAxisValue(uint32_t tag, float value);
    void setAxisValue(const char* tagStr, float value);

    /**
     * Get current axis value
     */
    float getAxisValue(uint32_t tag) const;
    float getAxisValue(const char* tagStr) const;

    /**
     * Apply named instance preset
     */
    bool applyInstance(const std::string& instanceName);
    bool applyInstance(size_t instanceIndex);

    /**
     * Reset all axes to default values
     */
    void resetToDefaults();

    /**
     * Get interpolated glyph outline at current axis values
     * @param parser Source parser with glyph data
     * @param glyphID Glyph to interpolate
     * @return Interpolated outline (caller must handle)
     */
    GlyphOutline getInterpolatedOutline(TTFParser& parser, uint16_t glyphID) const;

    /**
     * Get normalized axis coordinate (-1 to 1)
     */
    float normalizeAxisValue(uint32_t tag, float value) const;

    /**
     * Apply AVAR mapping to normalized coordinate
     */
    float applyAvarMapping(uint32_t tag, float normalizedValue) const;

  private:
    // Parse FVAR (font variations) table
    bool parseFvarTable(BinaryStream& stream, const TableDirectory& table);

    // Parse AVAR (axis variations) table
    bool parseAvarTable(BinaryStream& stream, const TableDirectory& table);

    // Parse GVAR (glyph variations) table
    bool parseGvarTable(BinaryStream& stream, const TableDirectory& table);

    // Interpolate point deltas for current axis values
    void interpolateDeltas(const GlyphVariation& variation, std::vector<std::pair<int16_t, int16_t>>& outDeltas) const;

    // Convert 4-char string to tag
    static uint32_t makeTag(const char* str);

    std::vector<FontAxis> axes_;
    std::vector<FontInstance> instances_;
    std::unordered_map<uint32_t, std::vector<AxisValueMap>> avarMappings_;
    std::vector<GlyphVariation> glyphVariations_;

    // Current axis values
    std::unordered_map<uint32_t, float> currentValues_;
};

} // namespace dakt::gui

#endif
