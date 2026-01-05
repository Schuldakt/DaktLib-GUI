#include "dakt/gui/text/GlyphAtlas.hpp"
#include "dakt/gui/text/Font.hpp"
#include "dakt/gui/text/SDFGenerator.hpp"
#include "dakt/gui/text/TTFParser.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>

namespace dakt::gui {

// ============================================================================
// Skyline Bin Packer
// ============================================================================

struct SkylineNode {
    uint32_t x, y, width;
};

class SkylinePacker {
  public:
    SkylinePacker(uint32_t width, uint32_t height) : width_(width), height_(height) { reset(); }

    void reset() {
        skyline_.clear();
        skyline_.push_back({0, 0, width_});
    }

    bool pack(uint32_t rectWidth, uint32_t rectHeight, uint32_t& outX, uint32_t& outY) {
        // Find best position using bottom-left heuristic
        int bestIdx = -1;
        uint32_t bestY = height_;
        uint32_t bestWidth = width_;

        for (size_t i = 0; i < skyline_.size(); ++i) {
            uint32_t y = fitSkyline(i, rectWidth, rectHeight);
            if (y != UINT32_MAX) {
                if (y < bestY || (y == bestY && skyline_[i].width < bestWidth)) {
                    bestIdx = static_cast<int>(i);
                    bestY = y;
                    bestWidth = skyline_[i].width;
                }
            }
        }

        if (bestIdx == -1)
            return false;

        // Place the rectangle
        outX = skyline_[bestIdx].x;
        outY = bestY;

        // Add new skyline node
        SkylineNode newNode;
        newNode.x = outX;
        newNode.y = outY + rectHeight;
        newNode.width = rectWidth;

        skyline_.insert(skyline_.begin() + bestIdx, newNode);

        // Merge and shrink overlapping nodes
        for (size_t i = bestIdx + 1; i < skyline_.size(); ++i) {
            if (skyline_[i].x < skyline_[i - 1].x + skyline_[i - 1].width) {
                uint32_t shrink = skyline_[i - 1].x + skyline_[i - 1].width - skyline_[i].x;
                skyline_[i].x += shrink;
                if (skyline_[i].width <= shrink) {
                    skyline_.erase(skyline_.begin() + i);
                    --i;
                } else {
                    skyline_[i].width -= shrink;
                    break;
                }
            } else {
                break;
            }
        }

        // Merge adjacent nodes with same height
        for (size_t i = 0; i < skyline_.size() - 1; ++i) {
            if (skyline_[i].y == skyline_[i + 1].y) {
                skyline_[i].width += skyline_[i + 1].width;
                skyline_.erase(skyline_.begin() + i + 1);
                --i;
            }
        }

        return true;
    }

  private:
    uint32_t fitSkyline(size_t nodeIdx, uint32_t rectWidth, uint32_t rectHeight) {
        uint32_t x = skyline_[nodeIdx].x;
        if (x + rectWidth > width_)
            return UINT32_MAX;

        uint32_t y = skyline_[nodeIdx].y;
        uint32_t widthLeft = rectWidth;
        size_t i = nodeIdx;

        while (widthLeft > 0) {
            if (i >= skyline_.size())
                return UINT32_MAX;

            y = std::max(y, skyline_[i].y);
            if (y + rectHeight > height_)
                return UINT32_MAX;

            widthLeft = (widthLeft > skyline_[i].width) ? widthLeft - skyline_[i].width : 0;
            ++i;
        }

        return y;
    }

    uint32_t width_, height_;
    std::vector<SkylineNode> skyline_;
};

// ============================================================================
// GlyphAtlas Implementation
// ============================================================================

GlyphAtlas::GlyphAtlas(uint32_t pageWidth, uint32_t pageHeight) : pageWidth_(pageWidth), pageHeight_(pageHeight) {
    // Create first page
    pages_.emplace_back();
    pages_[0].width = pageWidth_;
    pages_[0].height = pageHeight_;
    pages_[0].pixelData.resize(pageWidth_ * pageHeight_, 128); // Neutral SDF value
}

GlyphAtlas::~GlyphAtlas() = default;

static uint32_t hashFontSize(float fontSize) {
    // Hash font size to integer for map key
    return static_cast<uint32_t>(fontSize * 10.0f);
}

bool GlyphAtlas::addGlyph(Font& font, uint32_t glyphID, float fontSize) {
    auto key = std::make_pair(glyphID, hashFontSize(fontSize));
    if (glyphMap_.find(key) != glyphMap_.end()) {
        return true; // Already in atlas
    }

    // Get glyph outline from font's parser
    const Glyph* glyph = font.getGlyph(static_cast<uint16_t>(glyphID));
    if (!glyph)
        return false;

    // Generate SDF bitmap
    SDFGenerator generator;
    generator.setSpread(static_cast<float>(sdfSpread_));
    generator.setMode(enableMSDF_ ? SDFMode::MSDF : SDFMode::SDF);

    // We need access to the outline - get it through the font
    // For now, create a simple outline from glyph metrics
    GlyphOutline outline;
    outline.xMin = glyph->xMin;
    outline.yMin = glyph->yMin;
    outline.xMax = glyph->xMax;
    outline.yMax = glyph->yMax;
    outline.advanceWidth = glyph->advanceWidth;
    outline.leftSideBearing = glyph->leftSideBearing;

    // Note: Real implementation would get full contour data from TTFParser
    // For now, just allocate space in atlas

    float scale = fontSize / static_cast<float>(font.getUnitsPerEm());
    uint32_t glyphWidth = static_cast<uint32_t>(std::ceil((glyph->xMax - glyph->xMin) * scale)) + sdfSpread_ * 2 + 2;
    uint32_t glyphHeight = static_cast<uint32_t>(std::ceil((glyph->yMax - glyph->yMin) * scale)) + sdfSpread_ * 2 + 2;

    if (glyphWidth < 1)
        glyphWidth = 1;
    if (glyphHeight < 1)
        glyphHeight = 1;

    // Try to pack into existing pages
    uint32_t packX, packY;
    uint32_t pageIdx = 0;
    bool packed = false;

    for (pageIdx = 0; pageIdx < static_cast<uint32_t>(pages_.size()); ++pageIdx) {
        if (packGlyph(glyphWidth, glyphHeight, packX, packY)) {
            packed = true;
            break;
        }
    }

    if (!packed) {
        // Create new page
        AtlasPage newPage;
        newPage.width = pageWidth_;
        newPage.height = pageHeight_;
        newPage.pixelData.resize(pageWidth_ * pageHeight_, 128);
        pages_.push_back(newPage);

        // Reset packer for new page
        packRects_.clear();
        if (!packGlyph(glyphWidth, glyphHeight, packX, packY)) {
            return false; // Glyph too large for page
        }
        pageIdx = static_cast<uint32_t>(pages_.size()) - 1;
    }

    // Create atlas glyph entry
    AtlasGlyph atlasGlyph;
    atlasGlyph.glyphID = glyphID;
    atlasGlyph.fontSize = fontSize;
    atlasGlyph.atlasX = static_cast<float>(packX) / pageWidth_;
    atlasGlyph.atlasY = static_cast<float>(packY) / pageHeight_;
    atlasGlyph.atlasWidth = static_cast<float>(glyphWidth) / pageWidth_;
    atlasGlyph.atlasHeight = static_cast<float>(glyphHeight) / pageHeight_;
    atlasGlyph.advanceWidth = glyph->advanceWidth * scale;
    atlasGlyph.bearingX = glyph->leftSideBearing * scale;
    atlasGlyph.bearingY = glyph->yMax * scale;
    atlasGlyph.width = static_cast<float>(glyphWidth);
    atlasGlyph.height = static_cast<float>(glyphHeight);
    atlasGlyph.pageIndex = pageIdx;

    glyphMap_[key] = atlasGlyph;

    return true;
}

bool GlyphAtlas::hasGlyph(uint32_t glyphID, float fontSize) const {
    auto key = std::make_pair(glyphID, hashFontSize(fontSize));
    return glyphMap_.find(key) != glyphMap_.end();
}

const AtlasGlyph& GlyphAtlas::getGlyph(uint32_t glyphID, float fontSize) const {
    static AtlasGlyph empty{};
    auto key = std::make_pair(glyphID, hashFontSize(fontSize));
    auto it = glyphMap_.find(key);
    if (it != glyphMap_.end()) {
        return it->second;
    }
    return empty;
}

void GlyphAtlas::clear() {
    glyphMap_.clear();
    packRects_.clear();

    // Reset pages to initial state
    for (auto& page : pages_) {
        std::fill(page.pixelData.begin(), page.pixelData.end(), static_cast<uint8_t>(128));
    }

    // Keep only first page
    if (pages_.size() > 1) {
        pages_.resize(1);
    }
}

void GlyphAtlas::regenerate(Font& font, float fontSize) {
    clear();

    // Add all common ASCII glyphs
    for (uint32_t codepoint = 32; codepoint < 127; ++codepoint) {
        uint16_t glyphId = font.getGlyphId(codepoint);
        if (glyphId > 0) {
            addGlyph(font, glyphId, fontSize);
        }
    }
}

// ============================================================================
// Packing
// ============================================================================

bool GlyphAtlas::packGlyph(uint32_t width, uint32_t height, uint32_t& outX, uint32_t& outY) {
    // Simple row-based packing (could be upgraded to skyline later)
    static uint32_t currentX = 0;
    static uint32_t currentY = 0;
    static uint32_t rowHeight = 0;

    if (currentX + width > pageWidth_) {
        // Move to next row
        currentX = 0;
        currentY += rowHeight;
        rowHeight = 0;
    }

    if (currentY + height > pageHeight_) {
        // Page full
        currentX = 0;
        currentY = 0;
        rowHeight = 0;
        return false;
    }

    outX = currentX;
    outY = currentY;
    currentX += width;
    rowHeight = std::max(rowHeight, height);

    return true;
}

// ============================================================================
// File I/O
// ============================================================================

// .daktfont file format:
// Header: "DAKTFONT" (8 bytes)
// Version: uint32_t
// Page count: uint32_t
// Glyph count: uint32_t
// Page width: uint32_t
// Page height: uint32_t
// SDF spread: uint32_t
// [Pages]: For each page: raw pixel data
// [Glyphs]: For each glyph: AtlasGlyph struct

bool GlyphAtlas::saveToFile(const std::string& filePath) const {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open())
        return false;

    // Header
    const char magic[] = "DAKTFONT";
    file.write(magic, 8);

    uint32_t version = 1;
    uint32_t pageCount = static_cast<uint32_t>(pages_.size());
    uint32_t glyphCount = static_cast<uint32_t>(glyphMap_.size());

    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    file.write(reinterpret_cast<const char*>(&pageCount), sizeof(pageCount));
    file.write(reinterpret_cast<const char*>(&glyphCount), sizeof(glyphCount));
    file.write(reinterpret_cast<const char*>(&pageWidth_), sizeof(pageWidth_));
    file.write(reinterpret_cast<const char*>(&pageHeight_), sizeof(pageHeight_));
    file.write(reinterpret_cast<const char*>(&sdfSpread_), sizeof(sdfSpread_));

    // Write pages
    for (const auto& page : pages_) {
        file.write(reinterpret_cast<const char*>(page.pixelData.data()), page.pixelData.size());
    }

    // Write glyph entries
    for (const auto& [key, glyph] : glyphMap_) {
        file.write(reinterpret_cast<const char*>(&glyph), sizeof(AtlasGlyph));
    }

    return true;
}

bool GlyphAtlas::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
        return false;

    // Read header
    char magic[8];
    file.read(magic, 8);
    if (std::strncmp(magic, "DAKTFONT", 8) != 0)
        return false;

    uint32_t version, pageCount, glyphCount;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    file.read(reinterpret_cast<char*>(&pageCount), sizeof(pageCount));
    file.read(reinterpret_cast<char*>(&glyphCount), sizeof(glyphCount));
    file.read(reinterpret_cast<char*>(&pageWidth_), sizeof(pageWidth_));
    file.read(reinterpret_cast<char*>(&pageHeight_), sizeof(pageHeight_));
    file.read(reinterpret_cast<char*>(&sdfSpread_), sizeof(sdfSpread_));

    if (version != 1)
        return false;

    // Read pages
    pages_.clear();
    pages_.resize(pageCount);
    for (auto& page : pages_) {
        page.width = pageWidth_;
        page.height = pageHeight_;
        page.pixelData.resize(pageWidth_ * pageHeight_);
        file.read(reinterpret_cast<char*>(page.pixelData.data()), page.pixelData.size());
    }

    // Read glyph entries
    glyphMap_.clear();
    for (uint32_t i = 0; i < glyphCount; ++i) {
        AtlasGlyph glyph;
        file.read(reinterpret_cast<char*>(&glyph), sizeof(AtlasGlyph));
        auto key = std::make_pair(glyph.glyphID, hashFontSize(glyph.fontSize));
        glyphMap_[key] = glyph;
    }

    return true;
}

} // namespace dakt::gui
