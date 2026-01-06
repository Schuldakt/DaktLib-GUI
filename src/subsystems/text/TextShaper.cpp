#include "dakt/gui/subsystems/text/TextShaper.hpp"
#include "dakt/gui/subsystems/text/Font.hpp"
#include "dakt/gui/subsystems/text/TTFParser.hpp"
#include <algorithm>
#include <unordered_map>

namespace dakt::gui {

// ============================================================================
// Common OpenType Script/Language Tags
// ============================================================================

static constexpr uint32_t SCRIPT_LATN = 0x4C41544E; // 'LATN' Latin
static constexpr uint32_t SCRIPT_ARAB = 0x41524142; // 'ARAB' Arabic
static constexpr uint32_t SCRIPT_HEBR = 0x48454252; // 'HEBR' Hebrew
static constexpr uint32_t SCRIPT_CYRL = 0x4359524C; // 'CYRL' Cyrillic

// Common ligature sequences
struct LigatureRule {
    std::vector<uint16_t> components; // Input glyphs
    uint16_t ligatureGlyph;           // Output glyph
};

// Common kerning pairs
struct KernPair {
    uint16_t left;
    uint16_t right;
    int16_t adjustment;
};

// ============================================================================
// TextShaper Implementation
// ============================================================================

TextShaper::TextShaper() {
    // Enable default features
    features_ = {
        {0x6C696761, true}, // 'liga' standard ligatures
        {0x6B65726E, true}, // 'kern' kerning
        {0x636C6967, true}, // 'clig' contextual ligatures
    };
}

TextShaper::~TextShaper() = default;

// UTF-8 decode helper
static uint32_t decodeUTF8(const std::string& text, size_t& i) {
    uint32_t codepoint = 0;
    unsigned char c = text[i];

    if ((c & 0x80) == 0) {
        codepoint = c;
        ++i;
    } else if ((c & 0xE0) == 0xC0) {
        codepoint = (c & 0x1F) << 6;
        if (i + 1 < text.length()) {
            codepoint |= (text[i + 1] & 0x3F);
        }
        i += 2;
    } else if ((c & 0xF0) == 0xE0) {
        codepoint = (c & 0x0F) << 12;
        if (i + 1 < text.length()) {
            codepoint |= (text[i + 1] & 0x3F) << 6;
        }
        if (i + 2 < text.length()) {
            codepoint |= (text[i + 2] & 0x3F);
        }
        i += 3;
    } else if ((c & 0xF8) == 0xF0) {
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
        ++i;
    }

    return codepoint;
}

// Detect script from codepoint
static uint32_t detectScript(uint32_t codepoint) {
    // Arabic: U+0600-U+06FF, U+0750-U+077F, U+08A0-U+08FF
    if ((codepoint >= 0x0600 && codepoint <= 0x06FF) || (codepoint >= 0x0750 && codepoint <= 0x077F) || (codepoint >= 0x08A0 && codepoint <= 0x08FF)) {
        return SCRIPT_ARAB;
    }

    // Hebrew: U+0590-U+05FF
    if (codepoint >= 0x0590 && codepoint <= 0x05FF) {
        return SCRIPT_HEBR;
    }

    // Cyrillic: U+0400-U+04FF
    if (codepoint >= 0x0400 && codepoint <= 0x04FF) {
        return SCRIPT_CYRL;
    }

    // Default to Latin
    return SCRIPT_LATN;
}

// Check if script is right-to-left
static bool isRTLScript(uint32_t script) { return script == SCRIPT_ARAB || script == SCRIPT_HEBR; }

ShapedRun TextShaper::shape(Font& font, const std::string& text, uint32_t scriptTag) {
    ShapedRun run;
    run.scriptTag = scriptTag;

    // Convert UTF-8 text to codepoints and get glyphs
    std::vector<uint32_t> codepoints;
    for (size_t i = 0; i < text.length();) {
        uint32_t cp = decodeUTF8(text, i);
        codepoints.push_back(cp);
    }

    // Detect script if not specified
    if (scriptTag == 0 && !codepoints.empty()) {
        run.scriptTag = detectScript(codepoints[0]);
    }
    run.isRTL = isRTLScript(run.scriptTag);

    // Map codepoints to glyphs
    uint32_t cluster = 0;
    for (uint32_t codepoint : codepoints) {
        uint16_t glyphID = font.getGlyphId(codepoint);
        const Glyph* glyph = font.getGlyph(glyphID);

        ShapedGlyph shapedGlyph;
        shapedGlyph.glyphID = glyphID;
        shapedGlyph.cluster = cluster;
        shapedGlyph.xAdvance = glyph ? static_cast<float>(glyph->advanceWidth) : 0.0f;
        shapedGlyph.yAdvance = 0;
        shapedGlyph.xOffset = 0;
        shapedGlyph.yOffset = 0;

        run.glyphs.push_back(shapedGlyph);
        ++cluster;
    }

    // Apply OpenType features
    applyGSUB(font, run);
    applyGPOS(font, run);

    return run;
}

std::vector<ShapedRun> TextShaper::shapeBidi(Font& font, const std::string& text) {
    std::vector<ShapedRun> runs;

    // UAX #9 Bidirectional Algorithm (simplified)
    // For full implementation, would need complete Unicode BiDi tables

    std::vector<uint32_t> codepoints;
    std::vector<uint8_t> bidiTypes;  // 0 = L, 1 = R, 2 = neutral
    std::vector<uint8_t> bidiLevels; // Embedding level

    // Decode and classify
    for (size_t i = 0; i < text.length();) {
        uint32_t cp = decodeUTF8(text, i);
        codepoints.push_back(cp);

        // Simple classification
        uint8_t type = 2; // neutral
        if ((cp >= 0x0041 && cp <= 0x005A) || (cp >= 0x0061 && cp <= 0x007A)) {
            type = 0; // Latin letters = L
        } else if ((cp >= 0x0600 && cp <= 0x06FF) || (cp >= 0x0590 && cp <= 0x05FF)) {
            type = 1; // Arabic/Hebrew = R
        }
        bidiTypes.push_back(type);
    }

    // Simple level assignment (not full algorithm)
    uint8_t baseLevel = 0;
    for (uint8_t type : bidiTypes) {
        if (type == 1) {
            baseLevel = 1; // RTL paragraph
            break;
        }
    }

    bidiLevels.resize(codepoints.size(), baseLevel);
    for (size_t i = 0; i < bidiTypes.size(); ++i) {
        if (bidiTypes[i] == 1) {
            bidiLevels[i] = 1;
        } else if (bidiTypes[i] == 0) {
            bidiLevels[i] = 0;
        }
    }

    // Split into runs by level
    if (codepoints.empty()) {
        return runs;
    }

    size_t runStart = 0;
    uint8_t currentLevel = bidiLevels[0];

    for (size_t i = 1; i <= codepoints.size(); ++i) {
        if (i == codepoints.size() || bidiLevels[i] != currentLevel) {
            // Create run
            std::string runText;
            for (size_t j = runStart; j < i; ++j) {
                // Re-encode to UTF-8
                uint32_t cp = codepoints[j];
                if (cp < 0x80) {
                    runText += static_cast<char>(cp);
                } else if (cp < 0x800) {
                    runText += static_cast<char>(0xC0 | (cp >> 6));
                    runText += static_cast<char>(0x80 | (cp & 0x3F));
                } else if (cp < 0x10000) {
                    runText += static_cast<char>(0xE0 | (cp >> 12));
                    runText += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                    runText += static_cast<char>(0x80 | (cp & 0x3F));
                }
            }

            ShapedRun run = shape(font, runText, currentLevel == 1 ? SCRIPT_ARAB : SCRIPT_LATN);
            run.isRTL = (currentLevel % 2) == 1;

            // Reverse RTL runs
            if (run.isRTL) {
                std::reverse(run.glyphs.begin(), run.glyphs.end());
            }

            runs.push_back(run);

            if (i < codepoints.size()) {
                runStart = i;
                currentLevel = bidiLevels[i];
            }
        }
    }

    // Reorder runs according to levels
    std::vector<uint32_t> runLevels;
    for (const auto& run : runs) {
        runLevels.push_back(run.isRTL ? 1 : 0);
    }
    reorderLogicalToVisual(runs, runLevels);

    return runs;
}

void TextShaper::applyGSUB(Font& font, ShapedRun& run) {
    // Check if ligatures feature is enabled
    bool ligaturesEnabled = false;
    for (const auto& feature : features_) {
        if (feature.tag == 0x6C696761 && feature.enabled) { // 'liga'
            ligaturesEnabled = true;
            break;
        }
    }

    if (!ligaturesEnabled || run.glyphs.size() < 2) {
        return;
    }

    // Common Latin ligatures (hardcoded for now - real implementation parses GSUB table)
    // f + i -> fi (U+FB01), f + l -> fl (U+FB02), f + f -> ff, f + f + i -> ffi, etc.

    uint16_t glyphF = font.getGlyphId('f');
    uint16_t glyphI = font.getGlyphId('i');
    uint16_t glyphL = font.getGlyphId('l');
    uint16_t glyphFi = font.getGlyphId(0xFB01); // fi ligature
    uint16_t glyphFl = font.getGlyphId(0xFB02); // fl ligature

    if (glyphF == 0)
        return;

    // Scan for ligature sequences
    std::vector<ShapedGlyph> newGlyphs;
    newGlyphs.reserve(run.glyphs.size());

    for (size_t i = 0; i < run.glyphs.size(); ++i) {
        if (run.glyphs[i].glyphID == glyphF && i + 1 < run.glyphs.size()) {
            // Check for fi
            if (run.glyphs[i + 1].glyphID == glyphI && glyphFi != 0) {
                ShapedGlyph lig = run.glyphs[i];
                lig.glyphID = glyphFi;

                const Glyph* ligGlyph = font.getGlyph(static_cast<uint16_t>(glyphFi));
                if (ligGlyph) {
                    lig.xAdvance = static_cast<float>(ligGlyph->advanceWidth);
                }

                newGlyphs.push_back(lig);
                ++i; // Skip the 'i'
                continue;
            }
            // Check for fl
            if (run.glyphs[i + 1].glyphID == glyphL && glyphFl != 0) {
                ShapedGlyph lig = run.glyphs[i];
                lig.glyphID = glyphFl;

                const Glyph* ligGlyph = font.getGlyph(static_cast<uint16_t>(glyphFl));
                if (ligGlyph) {
                    lig.xAdvance = static_cast<float>(ligGlyph->advanceWidth);
                }

                newGlyphs.push_back(lig);
                ++i; // Skip the 'l'
                continue;
            }
        }

        newGlyphs.push_back(run.glyphs[i]);
    }

    run.glyphs = std::move(newGlyphs);
}

void TextShaper::applyGPOS(Font& font, ShapedRun& run) {
    // Check if kerning feature is enabled
    bool kerningEnabled = false;
    for (const auto& feature : features_) {
        if (feature.tag == 0x6B65726E && feature.enabled) { // 'kern'
            kerningEnabled = true;
            break;
        }
    }

    if (!kerningEnabled || run.glyphs.size() < 2) {
        return;
    }

    // Apply kerning pairs (real implementation parses GPOS/kern tables)
    // For now, use hardcoded common pairs

    // Common kerning pairs (in font units, typical values)
    static const std::unordered_map<uint32_t, int16_t> commonKernPairs = {
        // A + V, A + W, A + Y combinations
        {('A' << 16) | 'V', -80},
        {('A' << 16) | 'W', -60},
        {('A' << 16) | 'Y', -80},
        {('A' << 16) | 'T', -80},

        // T + a, T + o combinations
        {('T' << 16) | 'a', -80},
        {('T' << 16) | 'o', -80},
        {('T' << 16) | 'e', -60},

        // V + a, V + e, V + o
        {('V' << 16) | 'a', -60},
        {('V' << 16) | 'e', -50},
        {('V' << 16) | 'o', -50},

        // W + a, W + e
        {('W' << 16) | 'a', -40},
        {('W' << 16) | 'e', -30},

        // Y + a, Y + e, Y + o
        {('Y' << 16) | 'a', -80},
        {('Y' << 16) | 'e', -70},
        {('Y' << 16) | 'o', -70},
    };

    // Build glyph-to-char map for kern lookup
    std::unordered_map<uint16_t, char> glyphToChar;
    for (char c = 'A'; c <= 'Z'; ++c) {
        glyphToChar[font.getGlyphId(c)] = c;
    }
    for (char c = 'a'; c <= 'z'; ++c) {
        glyphToChar[font.getGlyphId(c)] = c;
    }

    // Apply kerning
    for (size_t i = 0; i + 1 < run.glyphs.size(); ++i) {
        auto it1 = glyphToChar.find(static_cast<uint16_t>(run.glyphs[i].glyphID));
        auto it2 = glyphToChar.find(static_cast<uint16_t>(run.glyphs[i + 1].glyphID));

        if (it1 != glyphToChar.end() && it2 != glyphToChar.end()) {
            uint32_t pairKey = (static_cast<uint32_t>(it1->second) << 16) | it2->second;
            auto kernIt = commonKernPairs.find(pairKey);
            if (kernIt != commonKernPairs.end()) {
                // Apply kern value (scaled by font units)
                run.glyphs[i].xAdvance += kernIt->second;
            }
        }
    }
}

void TextShaper::substituteWhitespace(ShapedRun& run, uint32_t spaceGlyphID) {
    for (auto& glyph : run.glyphs) {
        // Check if this is a space character (various Unicode spaces)
        // In a real implementation, would check cluster's original codepoint
        if (glyph.glyphID == 0) {
            glyph.glyphID = spaceGlyphID;
        }
    }
}

void TextShaper::determineBidiLevel(const std::string& text, std::vector<uint32_t>& levels) {
    // Simplified - real implementation follows UAX #9
    levels.clear();
    for (size_t i = 0; i < text.length();) {
        uint32_t cp = decodeUTF8(text, i);

        // RTL characters get level 1
        if ((cp >= 0x0590 && cp <= 0x05FF) || (cp >= 0x0600 && cp <= 0x06FF)) {
            levels.push_back(1);
        } else {
            levels.push_back(0);
        }
    }
}

void TextShaper::reorderLogicalToVisual(std::vector<ShapedRun>& runs, const std::vector<uint32_t>& levels) {
    if (runs.size() <= 1)
        return;

    // Find maximum level
    uint32_t maxLevel = 0;
    for (uint32_t level : levels) {
        maxLevel = std::max(maxLevel, level);
    }

    // Reverse subsequences at each level (from highest to lowest)
    for (uint32_t level = maxLevel; level >= 1; --level) {
        size_t i = 0;
        while (i < runs.size()) {
            // Find start of run at this level
            while (i < runs.size() && levels[i] < level) {
                ++i;
            }

            size_t start = i;

            // Find end of run at this level
            while (i < runs.size() && levels[i] >= level) {
                ++i;
            }

            // Reverse this subsequence
            if (start < i) {
                std::reverse(runs.begin() + start, runs.begin() + i);
            }
        }
    }
}

} // namespace dakt::gui
