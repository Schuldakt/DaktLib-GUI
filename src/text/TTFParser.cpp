#include "dakt/gui/text/TTFParser.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <vector>

namespace dakt::gui::text {

// ============================================================================
// BinaryStream Implementation
// ============================================================================

BinaryStream::BinaryStream(const std::vector<uint8_t>& data) : data_(data), position_(0) {}

BinaryStream::BinaryStream(const uint8_t* data, size_t size) : data_(data, data + size), position_(0) {}

uint8_t BinaryStream::readU8() {
    if (position_ >= data_.size())
        return 0;
    return data_[position_++];
}

int8_t BinaryStream::readI8() { return static_cast<int8_t>(readU8()); }

uint16_t BinaryStream::readU16() {
    uint16_t value = (static_cast<uint16_t>(readU8()) << 8) | readU8();
    return value;
}

int16_t BinaryStream::readI16() { return static_cast<int16_t>(readU16()); }

uint32_t BinaryStream::readU32() {
    uint32_t value = (static_cast<uint32_t>(readU16()) << 16) | readU16();
    return value;
}

int32_t BinaryStream::readI32() { return static_cast<int32_t>(readU32()); }

std::vector<uint8_t> BinaryStream::readBytes(size_t count) {
    std::vector<uint8_t> result;
    for (size_t i = 0; i < count && position_ < data_.size(); ++i) {
        result.push_back(data_[position_++]);
    }
    return result;
}

void BinaryStream::seek(size_t offset) { position_ = std::min(offset, data_.size()); }

void BinaryStream::skip(size_t count) { position_ = std::min(position_ + count, data_.size()); }

// ============================================================================
// TTFParser Implementation
// ============================================================================

bool TTFParser::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return false;

    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    fontData_.resize(fileSize);
    file.read(reinterpret_cast<char*>(fontData_.data()), fileSize);
    file.close();

    return loadFromMemory(fontData_.data(), fontData_.size());
}

bool TTFParser::loadFromMemory(const uint8_t* data, size_t size) {
    if (!data || size < 12)
        return false;

    fontData_ = std::vector<uint8_t>(data, data + size);
    BinaryStream stream(fontData_);

    // Parse offset table (sfnt wrapper)
    if (!parseOffsetTable(stream))
        return false;

    // Parse required tables
    if (!parseHeadTable(stream))
        return false;
    if (!parseHheaTable(stream))
        return false;
    if (!parseMaxpTable(stream))
        return false;
    if (!parseLocaTable(stream))
        return false;
    if (!parseGlyfTable(stream))
        return false;
    if (!parseHmtxTable(stream))
        return false;
    if (!parseCmapTable(stream))
        return false;
    if (!parseNameTable(stream))
        return false;

    return true;
}

bool TTFParser::parseOffsetTable(BinaryStream& stream) {
    uint32_t version = stream.readU32();
    // version can be 0x00010000 (v1.0) or 0x4F54544F ('OTTO' for CFF)

    uint16_t numTables = stream.readU16();
    stream.skip(6); // searchRange, entrySelector, rangeShift

    tables_.clear();
    tables_.resize(numTables);

    for (uint16_t i = 0; i < numTables; ++i) {
        tables_[i].tag = stream.readU32();
        tables_[i].checksum = stream.readU32();
        tables_[i].offset = stream.readU32();
        tables_[i].length = stream.readU32();
    }

    return !tables_.empty();
}

bool TTFParser::parseHeadTable(BinaryStream& stream) {
    const TableDirectory* headTable = findTable(0x68656164); // 'head'
    if (!headTable)
        return false;

    stream.seek(headTable->offset);

    stream.skip(4); // tableVersion
    stream.skip(4); // fontRevision
    stream.skip(4); // checkSumAdjustment
    stream.skip(4); // magicNumber

    fontMetrics_.flags = stream.readU16();
    fontMetrics_.unitsPerEm = stream.readU16();

    fontMetrics_.created = stream.readU32();
    fontMetrics_.modified = stream.readU32();

    fontMetrics_.xMin = stream.readI16();
    fontMetrics_.yMin = stream.readI16();
    fontMetrics_.xMax = stream.readI16();
    fontMetrics_.yMax = stream.readI16();

    fontMetrics_.macStyle = stream.readU16();

    stream.skip(2); // lowestRecPPEM
    stream.skip(2); // indexToLocFormat
    isShortLocaFormat_ = (stream.readI16() == 0);

    return true;
}

bool TTFParser::parseHheaTable(BinaryStream& stream) {
    const TableDirectory* hheaTable = findTable(0x68686561); // 'hhea'
    if (!hheaTable)
        return false;

    stream.seek(hheaTable->offset);

    stream.skip(4); // tableVersion
    hMetrics_.ascender = stream.readI16();
    hMetrics_.descender = stream.readI16();
    hMetrics_.lineGap = stream.readI16();
    hMetrics_.advanceWidthMax = stream.readU16();
    hMetrics_.minLeftSideBearing = stream.readI16();
    hMetrics_.minRightSideBearing = stream.readI16();
    hMetrics_.xMaxExtent = stream.readI16();
    hMetrics_.caretSlopeRise = stream.readI16();
    hMetrics_.caretSlopeRun = stream.readI16();

    stream.skip(12); // reserved
    numberOfHMetrics_ = stream.readU16();

    return true;
}

bool TTFParser::parseMaxpTable(BinaryStream& stream) {
    const TableDirectory* maxpTable = findTable(0x6D617870); // 'maxp'
    if (!maxpTable)
        return false;

    stream.seek(maxpTable->offset);
    stream.skip(4); // version
    glyphCount_ = stream.readU16();

    return true;
}

bool TTFParser::parseLocaTable(BinaryStream& stream) {
    const TableDirectory* locaTable = findTable(0x6C6F6361); // 'loca'
    if (!locaTable)
        return false;

    stream.seek(locaTable->offset);

    glyphLocations_.clear();
    glyphLocations_.resize(glyphCount_ + 1);

    for (uint16_t i = 0; i <= glyphCount_; ++i) {
        if (isShortLocaFormat_) {
            glyphLocations_[i] = static_cast<uint32_t>(stream.readU16()) * 2;
        } else {
            glyphLocations_[i] = stream.readU32();
        }
    }

    return true;
}

bool TTFParser::parseGlyfTable(BinaryStream& stream) {
    const TableDirectory* glyfTable = findTable(0x67006C6F); // 'glyf'
    if (!glyfTable)
        return false;

    // We parse glyphs on-demand in getGlyphOutline()
    // Just validate the table exists
    return true;
}

bool TTFParser::parseHmtxTable(BinaryStream& stream) {
    const TableDirectory* hmtxTable = findTable(0x686D7478); // 'hmtx'
    if (!hmtxTable)
        return false;

    stream.seek(hmtxTable->offset);

    advanceWidths_.resize(glyphCount_);
    leftSideBearings_.resize(glyphCount_);

    // Read hMetrics for numberOfHMetrics glyphs
    for (uint16_t i = 0; i < numberOfHMetrics_ && i < glyphCount_; ++i) {
        advanceWidths_[i] = stream.readU16();
        leftSideBearings_[i] = stream.readI16();
    }

    // Remaining glyphs use last advanceWidth but have individual LSB
    if (numberOfHMetrics_ < glyphCount_) {
        int16_t lastAdvance = advanceWidths_[numberOfHMetrics_ - 1];
        for (uint16_t i = numberOfHMetrics_; i < glyphCount_; ++i) {
            advanceWidths_[i] = lastAdvance;
            leftSideBearings_[i] = stream.readI16();
        }
    }

    return true;
}

bool TTFParser::parseCmapTable(BinaryStream& stream) {
    const TableDirectory* cmapTable = findTable(0x636D6170); // 'cmap'
    if (!cmapTable)
        return false;

    stream.seek(cmapTable->offset);

    stream.skip(2); // version
    uint16_t numSubtables = stream.readU16();

    // Look for platform 3 (Windows), encoding 1 (Unicode BMP)
    uint32_t format4Offset = 0;

    for (uint16_t i = 0; i < numSubtables; ++i) {
        uint16_t platformId = stream.readU16();
        uint16_t encodingId = stream.readU16();
        uint32_t offset = stream.readU32();

        if (platformId == 3 && (encodingId == 1 || encodingId == 10)) {
            format4Offset = cmapTable->offset + offset;
            break;
        }
    }

    if (format4Offset == 0)
        return false;

    stream.seek(format4Offset);
    uint16_t format = stream.readU16();

    if (format != 4)
        return false; // Only support format 4 for now

    stream.skip(2); // length
    stream.skip(2); // language

    uint16_t segCountX2 = stream.readU16();
    uint16_t segCount = segCountX2 / 2;

    stream.skip(6); // searchRange, entrySelector, rangeShift

    std::vector<uint16_t> endCode(segCount);
    std::vector<uint16_t> startCode(segCount);
    std::vector<int16_t> idDelta(segCount);
    std::vector<uint16_t> idRangeOffsets(segCount);

    for (uint16_t i = 0; i < segCount; ++i) {
        endCode[i] = stream.readU16();
    }
    stream.skip(2); // reservedPad

    for (uint16_t i = 0; i < segCount; ++i) {
        startCode[i] = stream.readU16();
    }

    for (uint16_t i = 0; i < segCount; ++i) {
        idDelta[i] = stream.readI16();
    }

    uint32_t idRangeOffsetsPos = stream.tell();
    for (uint16_t i = 0; i < segCount; ++i) {
        idRangeOffsets[i] = stream.readU16();
    }

    cmapEntries_.clear();

    for (uint16_t i = 0; i < segCount; ++i) {
        for (uint16_t code = startCode[i]; code <= endCode[i]; ++code) {
            uint16_t glyphId = 0;

            if (idRangeOffsets[i] == 0) {
                glyphId = (code + idDelta[i]) & 0xFFFF;
            } else {
                uint32_t glyphIndexPtr = idRangeOffsetsPos + (2 * i) + idRangeOffsets[i] + 2 * (code - startCode[i]);
                if (glyphIndexPtr < cmapTable->offset + cmapTable->length) {
                    stream.seek(glyphIndexPtr);
                    glyphId = stream.readU16();
                    if (glyphId != 0) {
                        glyphId = (glyphId + idDelta[i]) & 0xFFFF;
                    }
                }
            }

            if (glyphId > 0 && glyphId < glyphCount_) {
                cmapEntries_.push_back({code, glyphId});
            }
        }
    }

    return !cmapEntries_.empty();
}

bool TTFParser::parseNameTable(BinaryStream& stream) {
    const TableDirectory* nameTable = findTable(0x6E616D65); // 'name'
    if (!nameTable)
        return false;

    stream.seek(nameTable->offset);

    stream.skip(2); // version
    uint16_t count = stream.readU16();
    uint16_t stringOffset = stream.readU16();

    uint32_t nameTableBase = nameTable->offset;

    for (uint16_t i = 0; i < count; ++i) {
        uint16_t platformId = stream.readU16();
        uint16_t encodingId = stream.readU16();
        uint16_t languageId = stream.readU16();
        uint16_t nameId = stream.readU16();
        uint16_t length = stream.readU16();
        uint16_t offset = stream.readU16();

        // nameId 4 = Full Font Name, nameId 1 = Family Name
        if (platformId == 3 && (nameId == 4 || nameId == 1)) {
            std::string name = extractNameString(fontData_, nameTableBase + stringOffset + offset, length, platformId);
            if (nameId == 4)
                fullName_ = name;
            if (nameId == 1)
                familyName_ = name;
        }
    }

    return true;
}

GlyphOutline TTFParser::parseSimpleGlyph(BinaryStream& stream, int16_t xMin, int16_t yMin, int16_t xMax, int16_t yMax) {
    GlyphOutline outline;
    outline.xMin = xMin;
    outline.yMin = yMin;
    outline.xMax = xMax;
    outline.yMax = yMax;

    int16_t numberOfContours = stream.readI16();
    if (numberOfContours <= 0)
        return outline;

    std::vector<uint16_t> endPtsOfContours(numberOfContours);
    uint16_t totalPoints = 0;

    for (int16_t i = 0; i < numberOfContours; ++i) {
        endPtsOfContours[i] = stream.readU16();
        totalPoints = std::max(totalPoints, (uint16_t)(endPtsOfContours[i] + 1));
    }

    uint16_t instructionLength = stream.readU16();
    stream.skip(instructionLength); // Skip glyph program

    // Read flags
    std::vector<uint8_t> flags(totalPoints);
    for (uint16_t i = 0; i < totalPoints; ++i) {
        flags[i] = stream.readU8();
        if (flags[i] & 0x08) { // Repeat flag
            uint8_t count = stream.readU8();
            for (uint8_t j = 0; j < count && i + 1 < totalPoints; ++j) {
                flags[++i] = flags[i - 1];
            }
        }
    }

    // Read coordinates
    std::vector<int16_t> xCoordinates(totalPoints);
    std::vector<int16_t> yCoordinates(totalPoints);

    int16_t x = 0;
    for (uint16_t i = 0; i < totalPoints; ++i) {
        if (flags[i] & 0x02) { // X short
            int16_t dx = stream.readU8();
            if (!(flags[i] & 0x10))
                dx = -dx;
            x += dx;
        } else if (!(flags[i] & 0x10)) {
            x += stream.readI16();
        }
        xCoordinates[i] = x;
    }

    int16_t y = 0;
    for (uint16_t i = 0; i < totalPoints; ++i) {
        if (flags[i] & 0x04) { // Y short
            int16_t dy = stream.readU8();
            if (!(flags[i] & 0x20))
                dy = -dy;
            y += dy;
        } else if (!(flags[i] & 0x20)) {
            y += stream.readI16();
        }
        yCoordinates[i] = y;
    }

    // Build contours
    for (int16_t contourIdx = 0; contourIdx < numberOfContours; ++contourIdx) {
        GlyphContour contour;
        uint16_t startPoint = (contourIdx == 0) ? 0 : (endPtsOfContours[contourIdx - 1] + 1);
        uint16_t endPoint = endPtsOfContours[contourIdx];

        for (uint16_t i = startPoint; i <= endPoint; ++i) {
            GlyphPoint point;
            point.x = xCoordinates[i];
            point.y = yCoordinates[i];
            point.onCurve = (flags[i] & 0x01) != 0;
            contour.points.push_back(point);
        }

        outline.contours.push_back(contour);
    }

    return outline;
}

GlyphOutline TTFParser::parseCompositeGlyph(BinaryStream& stream, int16_t xMin, int16_t yMin, int16_t xMax, int16_t yMax) {
    GlyphOutline outline;
    outline.xMin = xMin;
    outline.yMin = yMin;
    outline.xMax = xMax;
    outline.yMax = yMax;

    // For now, return empty composite (would need recursive outline merging)
    return outline;
}

std::string TTFParser::extractNameString(const std::vector<uint8_t>& data, uint16_t offset, uint16_t length, uint16_t platformId) {
    if (offset + length > data.size())
        return "";

    std::string result;

    if (platformId == 3) { // Windows Unicode
        for (uint16_t i = 0; i < length - 1; i += 2) {
            uint16_t codepoint = (data[offset + i] << 8) | data[offset + i + 1];
            if (codepoint < 128) {
                result += static_cast<char>(codepoint);
            }
        }
    } else {
        for (uint16_t i = 0; i < length; ++i) {
            char ch = data[offset + i];
            if (ch >= 32 && ch < 127) {
                result += ch;
            }
        }
    }

    return result;
}

const TableDirectory* TTFParser::findTable(uint32_t tag) const {
    for (const auto& table : tables_) {
        if (table.tag == tag) {
            return &table;
        }
    }
    return nullptr;
}

uint16_t TTFParser::getGlyphId(uint32_t codepoint) const {
    for (const auto& entry : cmapEntries_) {
        if (entry.codepoint == codepoint) {
            return entry.glyphId;
        }
    }
    return 0;
}

const GlyphOutline* TTFParser::getGlyphOutline(uint16_t glyphId) const {
    if (glyphId >= glyphCount_)
        return nullptr;

    auto it = glyphCache_.find(glyphId);
    if (it != glyphCache_.end()) {
        return &it->second;
    }

    const TableDirectory* glyfTable = findTable(0x67006C6F); // 'glyf'
    if (!glyfTable)
        return nullptr;

    if (glyphId >= glyphLocations_.size() - 1)
        return nullptr;

    uint32_t glyphOffset = glyfTable->offset + glyphLocations_[glyphId];
    uint32_t glyphSize = glyphLocations_[glyphId + 1] - glyphLocations_[glyphId];

    if (glyphSize == 0)
        return nullptr;
    if (glyphOffset + glyphSize > fontData_.size())
        return nullptr;

    BinaryStream stream(fontData_.data() + glyphOffset, glyphSize);

    int16_t numberOfContours = stream.readI16();
    int16_t xMin = stream.readI16();
    int16_t yMin = stream.readI16();
    int16_t xMax = stream.readI16();
    int16_t yMax = stream.readI16();

    GlyphOutline outline;

    if (numberOfContours >= 0) {
        outline = const_cast<TTFParser*>(this)->parseSimpleGlyph(stream, xMin, yMin, xMax, yMax);
    } else {
        outline = const_cast<TTFParser*>(this)->parseCompositeGlyph(stream, xMin, yMin, xMax, yMax);
    }

    outline.advanceWidth = glyphId < advanceWidths_.size() ? advanceWidths_[glyphId] : 0;
    outline.leftSideBearing = glyphId < leftSideBearings_.size() ? leftSideBearings_[glyphId] : 0;

    auto& cached = const_cast<std::unordered_map<uint16_t, GlyphOutline>&>(glyphCache_)[glyphId];
    cached = outline;
    return &cached;
}

int16_t TTFParser::getAdvanceWidth(uint16_t glyphId) const { return glyphId < advanceWidths_.size() ? advanceWidths_[glyphId] : 0; }

int16_t TTFParser::getLeftSideBearing(uint16_t glyphId) const { return glyphId < leftSideBearings_.size() ? leftSideBearings_[glyphId] : 0; }

} // namespace dakt::gui::text