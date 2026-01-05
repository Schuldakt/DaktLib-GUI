#ifndef DAKT_GUI_OTF_PARSER_HPP
#define DAKT_GUI_OTF_PARSER_HPP

#include "TTFParser.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace dakt::gui {

/**
 * @brief CFF (Compact Font Format) glyph outline
 *
 * OpenType fonts can use either TrueType outlines (glyf table)
 * or CFF outlines. CFF uses cubic bezier curves instead of quadratic.
 */
struct CFFGlyph {
    std::vector<GlyphContour> contours;
    int16_t xMin = 0, yMin = 0, xMax = 0, yMax = 0;
    int16_t advanceWidth = 0;
    int16_t leftSideBearing = 0;
};

/**
 * @brief OpenType CFF table parser
 *
 * Parses CFF (Compact Font Format) outlines from OpenType fonts.
 * CFF fonts use PostScript Type 2 charstrings for glyph outlines.
 */
class OTFParser {
  public:
    OTFParser();
    ~OTFParser();

    /**
     * Parse CFF table from font data
     * @param fontData Raw font file data
     * @param tableOffset Offset to CFF table
     * @param tableLength Length of CFF table
     * @return true if parsing succeeded
     */
    bool parseCFF(const std::vector<uint8_t>& fontData, uint32_t tableOffset, uint32_t tableLength);

    /**
     * Check if CFF data is loaded
     */
    bool hasCFF() const { return hasCFF_; }

    /**
     * Get glyph outline from CFF data
     */
    CFFGlyph getGlyph(uint16_t glyphID) const;

    /**
     * Get number of glyphs in CFF font
     */
    uint16_t getGlyphCount() const { return glyphCount_; }

  private:
    // CFF Index structure
    struct CFFIndex {
        uint16_t count = 0;
        std::vector<uint32_t> offsets;
        std::vector<uint8_t> data;
    };

    // Parse CFF INDEX structure
    bool parseIndex(const uint8_t* data, size_t dataSize, size_t& offset, CFFIndex& index);

    // Parse Type 2 charstring
    CFFGlyph parseCharstring(const uint8_t* data, size_t length) const;

    // Execute Type 2 charstring operators
    void executeCharstring(const uint8_t* data, size_t length, CFFGlyph& glyph, std::vector<float>& stack) const;

    bool hasCFF_ = false;
    uint16_t glyphCount_ = 0;

    // CFF data structures
    CFFIndex nameIndex_;
    CFFIndex topDictIndex_;
    CFFIndex stringIndex_;
    CFFIndex charStringsIndex_;
    CFFIndex localSubrIndex_;
    CFFIndex globalSubrIndex_;

    // Top DICT values
    int16_t defaultWidthX_ = 0;
    int16_t nominalWidthX_ = 0;
};

} // namespace dakt::gui

#endif
