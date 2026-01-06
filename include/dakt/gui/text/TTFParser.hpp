#ifndef DAKTLIB_GUI_TEXT_TTFPARSER_HPP
#define DAKTLIB_GUI_TEXT_TTFPARSER_HPP

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace dakt::gui {

/**
 * @brief Binary stream reader for big-endian font data
 */
class BinaryStream {
  public:
    explicit BinaryStream(const std::vector<uint8_t>& data);
    explicit BinaryStream(const uint8_t* data, size_t size);

    uint8_t readU8();
    int8_t readI8();
    uint16_t readU16();
    int16_t readI16();
    uint32_t readU32();
    int32_t readI32();
    std::vector<uint8_t> readBytes(size_t count);

    void seek(size_t offset);
    void skip(size_t count);
    size_t tell() const { return position_; }
    size_t size() const { return data_.size(); }
    bool eof() const { return position_ >= data_.size(); }

  private:
    const std::vector<uint8_t> data_;
    size_t position_ = 0;
};

/**
 * @brief TrueType/OpenType font table directory entry
 */
struct TableDirectory {
    uint32_t tag;
    uint32_t checksum;
    uint32_t offset;
    uint32_t length;
};

/**
 * @brief TrueType glyph outline point
 */
struct GlyphPoint {
    int16_t x = 0;
    int16_t y = 0;
    bool onCurve = false;
};

/**
 * @brief TrueType glyph contour information
 */
struct GlyphContour {
    std::vector<GlyphPoint> points;
    int16_t xMin = 0;
    int16_t yMin = 0;
    int16_t xMax = 0;
    int16_t yMax = 0;
};

/**
 * @brief Parsed glyph outline data
 */
struct GlyphOutline {
    std::vector<GlyphContour> contours;
    int16_t xMin = 0;
    int16_t yMin = 0;
    int16_t xMax = 0;
    int16_t yMax = 0;
    int16_t advanceWidth = 0;
    int16_t leftSideBearing = 0;
};

/**
 * @brief Font metrics from HEAD table
 */
struct FontMetrics {
    int16_t unitsPerEm = 1000;
    int16_t xMin = 0;
    int16_t yMin = 0;
    int16_t xMax = 0;
    int16_t yMax = 0;
    uint16_t macStyle = 0;
    uint16_t flags = 0;
    uint32_t created = 0;
    uint32_t modified = 0;
};

/**
 * @brief Horizontal metrics from HHEA table
 */
struct HorizontalMetrics {
    int16_t ascender = 800;
    int16_t descender = -200;
    int16_t lineGap = 0;
    uint16_t advanceWidthMax = 0;
    int16_t minLeftSideBearing = 0;
    int16_t minRightSideBearing = 0;
    int16_t xMaxExtent = 0;
    int16_t caretSlopeRise = 1;
    int16_t caretSlopeRun = 0;
};

/**
 * @brief Character map format 4 entry (platform 3, encoding 1 / Unicode)
 */
struct CmapEntry {
    uint32_t codepoint = 0;
    uint16_t glyphId = 0;
};

/**
 * @brief TrueType/OpenType font parser
 * Supports TTF and OTF (with glyf outlines)
 */
class TTFParser {
  public:
    /**
     * Load and parse a TrueType/OpenType font file
     * @param filePath Path to .ttf or .otf file
     * @return true if parsing succeeded
     */
    bool loadFromFile(const std::string& filePath);

    /**
     * Load and parse font from memory buffer
     * @param data Binary font data
     * @param size Data size in bytes
     * @return true if parsing succeeded
     */
    bool loadFromMemory(const uint8_t* data, size_t size);

    // Table access
    const FontMetrics& getFontMetrics() const { return fontMetrics_; }
    const HorizontalMetrics& getHorizontalMetrics() const { return hMetrics_; }
    const std::vector<CmapEntry>& getCharacterMap() const { return cmapEntries_; }

    // Glyph queries
    uint16_t getGlyphId(uint32_t codepoint) const;
    const GlyphOutline* getGlyphOutline(uint16_t glyphId) const;
    int16_t getAdvanceWidth(uint16_t glyphId) const;
    int16_t getLeftSideBearing(uint16_t glyphId) const;

    // Font info
    uint16_t getGlyphCount() const { return glyphCount_; }
    const std::string& getFullName() const { return fullName_; }
    const std::string& getFamilyName() const { return familyName_; }

    // Raw data access (for variable font support)
    const std::vector<uint8_t>& getFontData() const { return fontData_; }
    const TableDirectory* findTable(uint32_t tag) const;

  private:
    // Parsing methods for each table
    bool parseOffsetTable(BinaryStream& stream);
    bool parseHeadTable(BinaryStream& stream);
    bool parseHheaTable(BinaryStream& stream);
    bool parseHmtxTable(BinaryStream& stream);
    bool parseMaxpTable(BinaryStream& stream);
    bool parseGlyfTable(BinaryStream& stream);
    bool parseLocaTable(BinaryStream& stream);
    bool parseCmapTable(BinaryStream& stream);
    bool parseNameTable(BinaryStream& stream);

    // Glyph outline parsing
    GlyphOutline parseSimpleGlyph(BinaryStream& stream, int16_t xMin, int16_t yMin, int16_t xMax, int16_t yMax);
    GlyphOutline parseCompositeGlyph(BinaryStream& stream, int16_t xMin, int16_t yMin, int16_t xMax, int16_t yMax);

    // Helper for name table string extraction
    std::string extractNameString(const std::vector<uint8_t>& data, uint16_t offset, uint16_t length, uint16_t platformId);

    // State
    std::vector<uint8_t> fontData_;
    std::vector<TableDirectory> tables_;
    std::vector<uint32_t> glyphLocations_; // loca table
    std::unordered_map<uint16_t, GlyphOutline> glyphCache_;
    std::vector<int16_t> advanceWidths_;    // hmtx table
    std::vector<int16_t> leftSideBearings_; // hmtx table

    FontMetrics fontMetrics_;
    HorizontalMetrics hMetrics_;
    std::vector<CmapEntry> cmapEntries_;

    uint16_t glyphCount_ = 0;
    uint16_t numberOfHMetrics_ = 0;
    bool isShortLocaFormat_ = false;

    std::string fullName_;
    std::string familyName_;
};

} // namespace dakt::gui

#endif