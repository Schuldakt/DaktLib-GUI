#include "dakt/gui/text/VariableFont.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace dakt::gui {

VariableFont::VariableFont() = default;
VariableFont::~VariableFont() = default;

uint32_t VariableFont::makeTag(const char* str) { return (static_cast<uint32_t>(str[0]) << 24) | (static_cast<uint32_t>(str[1]) << 16) | (static_cast<uint32_t>(str[2]) << 8) | static_cast<uint32_t>(str[3]); }

bool VariableFont::load(TTFParser& parser) {
    // Get font data stream
    const auto& fontData = parser.getFontData();
    if (fontData.empty())
        return false;

    BinaryStream stream(fontData);

    // Find FVAR table
    const TableDirectory* fvarTable = parser.findTable(makeTag("fvar"));
    if (!fvarTable) {
        // Not a variable font
        return false;
    }

    if (!parseFvarTable(stream, *fvarTable))
        return false;

    // Parse optional AVAR table
    const TableDirectory* avarTable = parser.findTable(makeTag("avar"));
    if (avarTable) {
        parseAvarTable(stream, *avarTable);
    }

    // Parse optional GVAR table
    const TableDirectory* gvarTable = parser.findTable(makeTag("gvar"));
    if (gvarTable) {
        parseGvarTable(stream, *gvarTable);
    }

    // Initialize current values to defaults
    resetToDefaults();

    return true;
}

bool VariableFont::parseFvarTable(BinaryStream& stream, const TableDirectory& table) {
    stream.seek(table.offset);

    uint16_t majorVersion = stream.readU16();
    uint16_t minorVersion = stream.readU16();
    (void)majorVersion;
    (void)minorVersion;

    uint16_t axisArrayOffset = stream.readU16();
    stream.skip(2); // reserved
    uint16_t axisCount = stream.readU16();
    uint16_t axisSize = stream.readU16();
    uint16_t instanceCount = stream.readU16();
    uint16_t instanceSize = stream.readU16();

    // Parse axes
    stream.seek(table.offset + axisArrayOffset);
    axes_.resize(axisCount);

    for (uint16_t i = 0; i < axisCount; ++i) {
        FontAxis& axis = axes_[i];
        axis.tag = stream.readU32();

        // Fixed-point 16.16 values
        int32_t minRaw = stream.readI32();
        int32_t defRaw = stream.readI32();
        int32_t maxRaw = stream.readI32();

        axis.minValue = static_cast<float>(minRaw) / 65536.0f;
        axis.defaultValue = static_cast<float>(defRaw) / 65536.0f;
        axis.maxValue = static_cast<float>(maxRaw) / 65536.0f;

        axis.flags = stream.readU16();
        axis.nameID = stream.readU16();

        // Convert tag to readable name
        char tagStr[5] = {static_cast<char>((axis.tag >> 24) & 0xFF), static_cast<char>((axis.tag >> 16) & 0xFF), static_cast<char>((axis.tag >> 8) & 0xFF), static_cast<char>(axis.tag & 0xFF), 0};
        axis.name = tagStr;

        // Skip any extra axis data
        if (axisSize > 20) {
            stream.skip(axisSize - 20);
        }
    }

    // Parse instances
    instances_.resize(instanceCount);
    for (uint16_t i = 0; i < instanceCount; ++i) {
        FontInstance& inst = instances_[i];
        inst.nameID = stream.readU16();
        stream.skip(2); // flags

        inst.coordinates.resize(axisCount);
        for (uint16_t j = 0; j < axisCount; ++j) {
            int32_t coordRaw = stream.readI32();
            inst.coordinates[j] = static_cast<float>(coordRaw) / 65536.0f;
        }

        // Skip any extra instance data (postScriptNameID if present)
        if (instanceSize > 4 + axisCount * 4) {
            stream.skip(instanceSize - 4 - axisCount * 4);
        }
    }

    return !axes_.empty();
}

bool VariableFont::parseAvarTable(BinaryStream& stream, const TableDirectory& table) {
    stream.seek(table.offset);

    uint16_t majorVersion = stream.readU16();
    uint16_t minorVersion = stream.readU16();
    (void)majorVersion;
    (void)minorVersion;

    stream.skip(2); // reserved
    uint16_t axisCount = stream.readU16();

    for (uint16_t i = 0; i < axisCount && i < axes_.size(); ++i) {
        uint16_t pairCount = stream.readU16();

        std::vector<AxisValueMap> mappings;
        mappings.reserve(pairCount);

        for (uint16_t j = 0; j < pairCount; ++j) {
            AxisValueMap map;
            int16_t fromRaw = stream.readI16();
            int16_t toRaw = stream.readI16();

            // F2Dot14 format (-2 to ~2 range)
            map.fromValue = static_cast<float>(fromRaw) / 16384.0f;
            map.toValue = static_cast<float>(toRaw) / 16384.0f;
            mappings.push_back(map);
        }

        avarMappings_[axes_[i].tag] = std::move(mappings);
    }

    return true;
}

bool VariableFont::parseGvarTable(BinaryStream& stream, const TableDirectory& table) {
    stream.seek(table.offset);

    uint16_t majorVersion = stream.readU16();
    uint16_t minorVersion = stream.readU16();
    (void)majorVersion;
    (void)minorVersion;

    uint16_t axisCount = stream.readU16();
    uint16_t sharedTupleCount = stream.readU16();
    uint32_t sharedTuplesOffset = stream.readU32();
    uint16_t glyphCount = stream.readU16();
    uint16_t flags = stream.readU16();
    uint32_t glyphVariationDataArrayOffset = stream.readU32();

    (void)axisCount;
    (void)sharedTupleCount;
    (void)sharedTuplesOffset;
    (void)flags;
    (void)glyphVariationDataArrayOffset;

    // Full GVAR parsing is complex - store offsets for on-demand parsing
    // For now, just validate the table exists
    glyphVariations_.clear();

    return glyphCount > 0;
}

const FontAxis* VariableFont::getAxis(uint32_t tag) const {
    for (const auto& axis : axes_) {
        if (axis.tag == tag)
            return &axis;
    }
    return nullptr;
}

const FontAxis* VariableFont::getAxis(const char* tagStr) const { return getAxis(makeTag(tagStr)); }

void VariableFont::setAxisValue(uint32_t tag, float value) {
    const FontAxis* axis = getAxis(tag);
    if (axis) {
        value = std::clamp(value, axis->minValue, axis->maxValue);
        currentValues_[tag] = value;
    }
}

void VariableFont::setAxisValue(const char* tagStr, float value) { setAxisValue(makeTag(tagStr), value); }

float VariableFont::getAxisValue(uint32_t tag) const {
    auto it = currentValues_.find(tag);
    if (it != currentValues_.end()) {
        return it->second;
    }
    const FontAxis* axis = getAxis(tag);
    return axis ? axis->defaultValue : 0.0f;
}

float VariableFont::getAxisValue(const char* tagStr) const { return getAxisValue(makeTag(tagStr)); }

bool VariableFont::applyInstance(const std::string& instanceName) {
    for (size_t i = 0; i < instances_.size(); ++i) {
        if (instances_[i].name == instanceName) {
            return applyInstance(i);
        }
    }
    return false;
}

bool VariableFont::applyInstance(size_t instanceIndex) {
    if (instanceIndex >= instances_.size())
        return false;

    const FontInstance& inst = instances_[instanceIndex];
    for (size_t i = 0; i < axes_.size() && i < inst.coordinates.size(); ++i) {
        currentValues_[axes_[i].tag] = inst.coordinates[i];
    }
    return true;
}

void VariableFont::resetToDefaults() {
    currentValues_.clear();
    for (const auto& axis : axes_) {
        currentValues_[axis.tag] = axis.defaultValue;
    }
}

float VariableFont::normalizeAxisValue(uint32_t tag, float value) const {
    const FontAxis* axis = getAxis(tag);
    if (!axis)
        return 0.0f;

    // Normalize to -1..0..1 range
    if (value < axis->defaultValue) {
        if (axis->defaultValue == axis->minValue)
            return 0.0f;
        return (value - axis->defaultValue) / (axis->defaultValue - axis->minValue);
    } else {
        if (axis->defaultValue == axis->maxValue)
            return 0.0f;
        return (value - axis->defaultValue) / (axis->maxValue - axis->defaultValue);
    }
}

float VariableFont::applyAvarMapping(uint32_t tag, float normalizedValue) const {
    auto it = avarMappings_.find(tag);
    if (it == avarMappings_.end())
        return normalizedValue;

    const auto& mappings = it->second;
    if (mappings.empty())
        return normalizedValue;

    // Find segment and interpolate
    for (size_t i = 1; i < mappings.size(); ++i) {
        if (normalizedValue <= mappings[i].fromValue) {
            float t = (normalizedValue - mappings[i - 1].fromValue) / (mappings[i].fromValue - mappings[i - 1].fromValue);
            return mappings[i - 1].toValue + t * (mappings[i].toValue - mappings[i - 1].toValue);
        }
    }

    return mappings.back().toValue;
}

GlyphOutline VariableFont::getInterpolatedOutline(TTFParser& parser, uint16_t glyphID) const {
    // Get base outline
    const GlyphOutline* baseOutline = parser.getGlyphOutline(glyphID);
    if (!baseOutline)
        return GlyphOutline{};

    GlyphOutline result = *baseOutline;

    // If no variations, return base outline
    if (!isVariable())
        return result;

    // Apply deltas from GVAR (simplified - full implementation would interpolate)
    // For each contour and point, apply accumulated deltas based on current axis values

    // Build normalized coordinate array
    std::vector<float> normalizedCoords;
    normalizedCoords.reserve(axes_.size());
    for (const auto& axis : axes_) {
        float value = getAxisValue(axis.tag);
        float normalized = normalizeAxisValue(axis.tag, value);
        normalized = applyAvarMapping(axis.tag, normalized);
        normalizedCoords.push_back(normalized);
    }

    // Find variation data for this glyph
    for (const auto& variation : glyphVariations_) {
        if (variation.glyphID != glyphID)
            continue;

        // Calculate scalar for this tuple
        float scalar = 1.0f;
        for (size_t i = 0; i < variation.tupleCoords.size() && i < normalizedCoords.size(); ++i) {
            if (variation.tupleCoords[i] != 0) {
                float coord = normalizedCoords[i];
                float peak = variation.tupleCoords[i];

                if ((peak > 0 && coord < 0) || (peak < 0 && coord > 0)) {
                    scalar = 0;
                    break;
                }

                scalar *= std::min(std::abs(coord) / std::abs(peak), 1.0f);
            }
        }

        if (scalar == 0)
            continue;

        // Apply deltas to points
        size_t deltaIdx = 0;
        for (auto& contour : result.contours) {
            for (auto& point : contour.points) {
                if (deltaIdx < variation.deltas.size()) {
                    point.x += static_cast<int16_t>(variation.deltas[deltaIdx].first * scalar);
                    point.y += static_cast<int16_t>(variation.deltas[deltaIdx].second * scalar);
                    ++deltaIdx;
                }
            }
        }
    }

    return result;
}

void VariableFont::interpolateDeltas(const GlyphVariation& variation, std::vector<std::pair<int16_t, int16_t>>& outDeltas) const {
    outDeltas = variation.deltas;

    // Calculate blend factor based on current axis values
    float blendFactor = 1.0f;

    for (size_t i = 0; i < axes_.size() && i < variation.tupleCoords.size(); ++i) {
        float current = getAxisValue(axes_[i].tag);
        float normalized = normalizeAxisValue(axes_[i].tag, current);
        float peak = variation.tupleCoords[i];

        if (peak != 0) {
            blendFactor *= std::clamp(normalized / peak, 0.0f, 1.0f);
        }
    }

    // Scale deltas
    for (auto& delta : outDeltas) {
        delta.first = static_cast<int16_t>(delta.first * blendFactor);
        delta.second = static_cast<int16_t>(delta.second * blendFactor);
    }
}

} // namespace dakt::gui
