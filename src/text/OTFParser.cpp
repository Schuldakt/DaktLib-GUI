#include "dakt/gui/text/OTFParser.hpp"
#include <algorithm>
#include <cstring>

namespace dakt::gui {

OTFParser::OTFParser() = default;
OTFParser::~OTFParser() = default;

bool OTFParser::parseIndex(const uint8_t* data, size_t dataSize, size_t& offset, CFFIndex& index) {
    if (offset + 2 > dataSize)
        return false;

    // Read count (2 bytes, big-endian)
    index.count = (data[offset] << 8) | data[offset + 1];
    offset += 2;

    if (index.count == 0) {
        return true; // Empty index is valid
    }

    if (offset + 1 > dataSize)
        return false;

    // Read offset size (1 byte)
    uint8_t offSize = data[offset++];
    if (offSize < 1 || offSize > 4)
        return false;

    // Read offsets
    index.offsets.resize(index.count + 1);
    for (uint16_t i = 0; i <= index.count; ++i) {
        if (offset + offSize > dataSize)
            return false;

        uint32_t off = 0;
        for (uint8_t j = 0; j < offSize; ++j) {
            off = (off << 8) | data[offset++];
        }
        index.offsets[i] = off;
    }

    // Calculate data size and copy
    if (!index.offsets.empty()) {
        size_t dataStart = offset;
        size_t indexDataSize = index.offsets.back() - 1; // Offsets are 1-based

        if (dataStart + indexDataSize > dataSize)
            return false;

        index.data.assign(data + dataStart, data + dataStart + indexDataSize);
        offset = dataStart + indexDataSize;
    }

    return true;
}

bool OTFParser::parseCFF(const std::vector<uint8_t>& fontData, uint32_t tableOffset, uint32_t tableLength) {
    if (tableOffset + tableLength > fontData.size())
        return false;

    const uint8_t* data = fontData.data() + tableOffset;
    size_t dataSize = tableLength;
    size_t offset = 0;

    // CFF Header
    if (offset + 4 > dataSize)
        return false;

    uint8_t major = data[offset++];
    uint8_t minor = data[offset++];
    uint8_t hdrSize = data[offset++];
    uint8_t offSize = data[offset++];

    (void)major;
    (void)minor;
    (void)offSize;

    // Skip to end of header
    offset = hdrSize;

    // Parse Name INDEX
    if (!parseIndex(data, dataSize, offset, nameIndex_))
        return false;

    // Parse Top DICT INDEX
    if (!parseIndex(data, dataSize, offset, topDictIndex_))
        return false;

    // Parse String INDEX
    if (!parseIndex(data, dataSize, offset, stringIndex_))
        return false;

    // Parse Global Subr INDEX
    if (!parseIndex(data, dataSize, offset, globalSubrIndex_))
        return false;

    // Parse Top DICT to find CharStrings and other data
    // (Simplified - full implementation would decode all DICT operators)
    if (topDictIndex_.count > 0 && !topDictIndex_.data.empty()) {
        // For now, just mark CFF as loaded
        hasCFF_ = true;

        // The charStrings offset would be parsed from Top DICT
        // This is a simplified version - real implementation parses DICT operators
    }

    return hasCFF_;
}

CFFGlyph OTFParser::getGlyph(uint16_t glyphID) const {
    CFFGlyph glyph;

    if (!hasCFF_ || glyphID >= charStringsIndex_.count)
        return glyph;

    // Get charstring data
    if (glyphID >= charStringsIndex_.offsets.size() - 1)
        return glyph;

    uint32_t startOffset = charStringsIndex_.offsets[glyphID] - 1; // 1-based
    uint32_t endOffset = charStringsIndex_.offsets[glyphID + 1] - 1;

    if (startOffset >= charStringsIndex_.data.size() || endOffset > charStringsIndex_.data.size())
        return glyph;

    size_t length = endOffset - startOffset;
    const uint8_t* charStringData = charStringsIndex_.data.data() + startOffset;

    return parseCharstring(charStringData, length);
}

CFFGlyph OTFParser::parseCharstring(const uint8_t* data, size_t length) const {
    CFFGlyph glyph;
    std::vector<float> stack;
    stack.reserve(48); // Max stack depth

    executeCharstring(data, length, glyph, stack);

    return glyph;
}

void OTFParser::executeCharstring(const uint8_t* data, size_t length, CFFGlyph& glyph, std::vector<float>& stack) const {
    // Type 2 Charstring interpreter (simplified)
    // Full implementation would handle all operators

    GlyphContour currentContour;
    float x = 0, y = 0; // Current point

    size_t i = 0;
    while (i < length) {
        uint8_t b0 = data[i++];

        if (b0 <= 11 || (b0 >= 13 && b0 <= 18) || b0 == 19 || b0 == 20 || (b0 >= 21 && b0 <= 31)) {
            // Operator
            switch (b0) {
            case 1:  // hstem
            case 3:  // vstem
            case 18: // hstemhm
            case 23: // vstemhm
                stack.clear();
                break;

            case 4: // vmoveto
                if (!stack.empty()) {
                    y += stack.back();
                    stack.clear();
                    if (!currentContour.points.empty()) {
                        glyph.contours.push_back(currentContour);
                        currentContour = GlyphContour();
                    }
                }
                break;

            case 5: // rlineto
                while (stack.size() >= 2) {
                    x += stack[0];
                    y += stack[1];
                    GlyphPoint pt;
                    pt.x = static_cast<int16_t>(x);
                    pt.y = static_cast<int16_t>(y);
                    pt.onCurve = true;
                    currentContour.points.push_back(pt);
                    stack.erase(stack.begin(), stack.begin() + 2);
                }
                break;

            case 6: // hlineto
                while (!stack.empty()) {
                    x += stack[0];
                    GlyphPoint pt;
                    pt.x = static_cast<int16_t>(x);
                    pt.y = static_cast<int16_t>(y);
                    pt.onCurve = true;
                    currentContour.points.push_back(pt);
                    stack.erase(stack.begin());
                    if (!stack.empty()) {
                        y += stack[0];
                        pt.x = static_cast<int16_t>(x);
                        pt.y = static_cast<int16_t>(y);
                        currentContour.points.push_back(pt);
                        stack.erase(stack.begin());
                    }
                }
                break;

            case 7: // vlineto
                while (!stack.empty()) {
                    y += stack[0];
                    GlyphPoint pt;
                    pt.x = static_cast<int16_t>(x);
                    pt.y = static_cast<int16_t>(y);
                    pt.onCurve = true;
                    currentContour.points.push_back(pt);
                    stack.erase(stack.begin());
                    if (!stack.empty()) {
                        x += stack[0];
                        pt.x = static_cast<int16_t>(x);
                        pt.y = static_cast<int16_t>(y);
                        currentContour.points.push_back(pt);
                        stack.erase(stack.begin());
                    }
                }
                break;

            case 8: // rrcurveto - cubic bezier
                while (stack.size() >= 6) {
                    // Control point 1
                    float x1 = x + stack[0];
                    float y1 = y + stack[1];
                    // Control point 2
                    float x2 = x1 + stack[2];
                    float y2 = y1 + stack[3];
                    // End point
                    x = x2 + stack[4];
                    y = y2 + stack[5];

                    GlyphPoint p1, p2, p3;
                    p1.x = static_cast<int16_t>(x1);
                    p1.y = static_cast<int16_t>(y1);
                    p1.onCurve = false;
                    p2.x = static_cast<int16_t>(x2);
                    p2.y = static_cast<int16_t>(y2);
                    p2.onCurve = false;
                    p3.x = static_cast<int16_t>(x);
                    p3.y = static_cast<int16_t>(y);
                    p3.onCurve = true;

                    currentContour.points.push_back(p1);
                    currentContour.points.push_back(p2);
                    currentContour.points.push_back(p3);

                    stack.erase(stack.begin(), stack.begin() + 6);
                }
                break;

            case 14: // endchar
                if (!currentContour.points.empty()) {
                    glyph.contours.push_back(currentContour);
                }
                return;

            case 21: // rmoveto
                if (stack.size() >= 2) {
                    x += stack[stack.size() - 2];
                    y += stack[stack.size() - 1];
                    stack.clear();
                    if (!currentContour.points.empty()) {
                        glyph.contours.push_back(currentContour);
                        currentContour = GlyphContour();
                    }
                }
                break;

            case 22: // hmoveto
                if (!stack.empty()) {
                    x += stack.back();
                    stack.clear();
                    if (!currentContour.points.empty()) {
                        glyph.contours.push_back(currentContour);
                        currentContour = GlyphContour();
                    }
                }
                break;

            default:
                // Unknown operator - clear stack
                stack.clear();
                break;
            }
        } else if (b0 == 12) {
            // Two-byte operator
            if (i < length) {
                uint8_t b1 = data[i++];
                // Handle two-byte operators (flex, etc.)
                (void)b1;
                stack.clear();
            }
        } else if (b0 >= 32 && b0 <= 246) {
            // Integer: b0 - 139
            stack.push_back(static_cast<float>(b0) - 139.0f);
        } else if (b0 >= 247 && b0 <= 250) {
            // Integer: (b0 - 247) * 256 + b1 + 108
            if (i < length) {
                uint8_t b1 = data[i++];
                stack.push_back(static_cast<float>((b0 - 247) * 256 + b1 + 108));
            }
        } else if (b0 >= 251 && b0 <= 254) {
            // Integer: -(b0 - 251) * 256 - b1 - 108
            if (i < length) {
                uint8_t b1 = data[i++];
                stack.push_back(static_cast<float>(-(b0 - 251) * 256 - b1 - 108));
            }
        } else if (b0 == 255) {
            // 4-byte float (16.16 fixed)
            if (i + 4 <= length) {
                int32_t val = (data[i] << 24) | (data[i + 1] << 16) | (data[i + 2] << 8) | data[i + 3];
                stack.push_back(static_cast<float>(val) / 65536.0f);
                i += 4;
            }
        } else if (b0 == 28) {
            // 2-byte integer
            if (i + 2 <= length) {
                int16_t val = (data[i] << 8) | data[i + 1];
                stack.push_back(static_cast<float>(val));
                i += 2;
            }
        }
    }

    // Close any remaining contour
    if (!currentContour.points.empty()) {
        glyph.contours.push_back(currentContour);
    }
}

} // namespace dakt::gui
