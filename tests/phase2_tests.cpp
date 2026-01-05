/**
 * @file phase2_tests.cpp
 * @brief Phase 2 unit tests for DaktLib-GUI text/font system
 *
 * Tests text parsing, SDF generation, glyph caching, and text shaping.
 */

#include "dakt/gui/core/Types.hpp"
#include "dakt/gui/text/Font.hpp"
#include "dakt/gui/text/GlyphAtlas.hpp"
#include "dakt/gui/text/GlyphCache.hpp"
#include "dakt/gui/text/OTFParser.hpp"
#include "dakt/gui/text/SDFGenerator.hpp"
#include "dakt/gui/text/TTFParser.hpp"
#include "dakt/gui/text/TextCursor.hpp"
#include "dakt/gui/text/TextShaper.hpp"
#include "dakt/gui/text/VariableFont.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace dakt::gui;

// ============================================================================
// Test Helpers
// ============================================================================

#define TEST(name)                                                                                                                                                                                                                                       \
    static void test_##name();                                                                                                                                                                                                                           \
    struct TestRunner_##name {                                                                                                                                                                                                                           \
        TestRunner_##name() {                                                                                                                                                                                                                            \
            printf("Testing %s...\n", #name);                                                                                                                                                                                                            \
            test_##name();                                                                                                                                                                                                                               \
            printf("✓ %s passed\n", #name);                                                                                                                                                                                                              \
        }                                                                                                                                                                                                                                                \
    };                                                                                                                                                                                                                                                   \
    static void test_##name()

#define ASSERT(cond)                                                                                                                                                                                                                                     \
    do {                                                                                                                                                                                                                                                 \
        if (!(cond)) {                                                                                                                                                                                                                                   \
            printf("FAILED: %s at %s:%d\n", #cond, __FILE__, __LINE__);                                                                                                                                                                                  \
            assert(false);                                                                                                                                                                                                                               \
        }                                                                                                                                                                                                                                                \
    } while (0)

#define ASSERT_NEAR(a, b, eps)                                                                                                                                                                                                                           \
    do {                                                                                                                                                                                                                                                 \
        if (std::abs((a) - (b)) > (eps)) {                                                                                                                                                                                                               \
            printf("FAILED: %s ~= %s (%.6f vs %.6f) at %s:%d\n", #a, #b, (double)(a), (double)(b), __FILE__, __LINE__);                                                                                                                                  \
            assert(false);                                                                                                                                                                                                                               \
        }                                                                                                                                                                                                                                                \
    } while (0)

// ============================================================================
// Vec3 Tests (new in Phase 2)
// ============================================================================

TEST(Vec3_operations) {
    Vec3 a(1.0f, 2.0f, 3.0f);
    Vec3 b(4.0f, 5.0f, 6.0f);

    // Addition
    Vec3 sum = a + b;
    ASSERT_NEAR(sum.x, 5.0f, 0.001f);
    ASSERT_NEAR(sum.y, 7.0f, 0.001f);
    ASSERT_NEAR(sum.z, 9.0f, 0.001f);

    // Subtraction
    Vec3 diff = b - a;
    ASSERT_NEAR(diff.x, 3.0f, 0.001f);
    ASSERT_NEAR(diff.y, 3.0f, 0.001f);
    ASSERT_NEAR(diff.z, 3.0f, 0.001f);

    // Scalar multiplication
    Vec3 scaled = a * 2.0f;
    ASSERT_NEAR(scaled.x, 2.0f, 0.001f);
    ASSERT_NEAR(scaled.y, 4.0f, 0.001f);
    ASSERT_NEAR(scaled.z, 6.0f, 0.001f);

    // Dot product
    float dot = a.dot(b);
    ASSERT_NEAR(dot, 32.0f, 0.001f); // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32

    // Cross product
    Vec3 cross = a.cross(b);
    ASSERT_NEAR(cross.x, -3.0f, 0.001f); // 2*6 - 3*5 = 12 - 15 = -3
    ASSERT_NEAR(cross.y, 6.0f, 0.001f);  // 3*4 - 1*6 = 12 - 6 = 6
    ASSERT_NEAR(cross.z, -3.0f, 0.001f); // 1*5 - 2*4 = 5 - 8 = -3

    // Length
    Vec3 c(3.0f, 4.0f, 0.0f);
    ASSERT_NEAR(c.length(), 5.0f, 0.001f);

    // Normalized
    Vec3 norm = c.normalized();
    ASSERT_NEAR(norm.length(), 1.0f, 0.001f);

    // Index access
    ASSERT_NEAR(a[0], 1.0f, 0.001f);
    ASSERT_NEAR(a[1], 2.0f, 0.001f);
    ASSERT_NEAR(a[2], 3.0f, 0.001f);
}

// ============================================================================
// BinaryStream Tests
// ============================================================================

TEST(BinaryStream_reading) {
    std::vector<uint8_t> data = {
        0x00, 0x01,             // U16: 1
        0x00, 0x00, 0x00, 0x0A, // U32: 10
        0xFF, 0xFE,             // I16: -2 (two's complement big-endian)
        0x41, 0x42, 0x43        // Bytes: "ABC"
    };

    BinaryStream stream(data);

    ASSERT(stream.readU16() == 1);
    ASSERT(stream.readU32() == 10);
    ASSERT(stream.readI16() == -2);

    auto bytes = stream.readBytes(3);
    ASSERT(bytes.size() == 3);
    ASSERT(bytes[0] == 'A');
    ASSERT(bytes[1] == 'B');
    ASSERT(bytes[2] == 'C');

    ASSERT(stream.eof());
}

TEST(BinaryStream_seeking) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
    BinaryStream stream(data);

    ASSERT(stream.tell() == 0);

    stream.seek(2);
    ASSERT(stream.tell() == 2);
    ASSERT(stream.readU8() == 0x03);

    stream.skip(1);
    ASSERT(stream.tell() == 4);
    ASSERT(stream.readU8() == 0x05);
}

// ============================================================================
// SDFGenerator Tests
// ============================================================================

TEST(SDFGenerator_basic) {
    SDFGenerator gen;

    // Test default settings
    ASSERT_NEAR(gen.getSpread(), 4.0f, 0.001f);
    ASSERT(gen.getMode() == SDFMode::SDF);

    // Test setting spread
    gen.setSpread(8.0f);
    ASSERT_NEAR(gen.getSpread(), 8.0f, 0.001f);

    // Test setting mode
    gen.setMode(SDFMode::MSDF);
    ASSERT(gen.getMode() == SDFMode::MSDF);
}

TEST(SDFGenerator_empty_glyph) {
    SDFGenerator gen;

    // Empty glyph (like space)
    GlyphOutline outline;
    outline.advanceWidth = 500;

    SDFGlyphBitmap bitmap = gen.generate(outline, 16.0f, 1000);

    ASSERT(bitmap.width == 1);
    ASSERT(bitmap.height == 1);
    ASSERT(bitmap.pixels.size() == 1);
    ASSERT(bitmap.pixels[0] == 128);                // Neutral SDF value
    ASSERT_NEAR(bitmap.advanceWidth, 8.0f, 0.001f); // 500 * 16 / 1000
}

TEST(SDFGenerator_shape_construction) {
    SDFGenerator gen;

    // Create a simple square glyph outline
    GlyphOutline outline;
    outline.xMin = 0;
    outline.yMin = 0;
    outline.xMax = 100;
    outline.yMax = 100;
    outline.advanceWidth = 120;

    GlyphContour contour;
    contour.points = {
        {0, 0, true},     // Bottom-left
        {100, 0, true},   // Bottom-right
        {100, 100, true}, // Top-right
        {0, 100, true}    // Top-left
    };
    outline.contours.push_back(contour);

    Shape shape = gen.outlineToShape(outline, 0.1f); // Scale to 10%

    ASSERT(shape.contours.size() == 1);
    ASSERT(!shape.contours[0].edges.empty());
}

// ============================================================================
// GlyphAtlas Tests
// ============================================================================

TEST(GlyphAtlas_construction) {
    GlyphAtlas atlas(512, 512);

    ASSERT(atlas.getPageCount() == 1);
    ASSERT(atlas.getPage(0).width == 512);
    ASSERT(atlas.getPage(0).height == 512);
}

TEST(GlyphAtlas_clear) {
    GlyphAtlas atlas(256, 256);

    // Clear should reset but keep one page
    atlas.clear();

    ASSERT(atlas.getPageCount() == 1);
}

TEST(GlyphAtlas_settings) {
    GlyphAtlas atlas(512, 512);

    atlas.setSDFSpread(4);
    atlas.setMSDF(true);

    // Settings don't affect page count
    ASSERT(atlas.getPageCount() >= 1);
}

// ============================================================================
// GlyphCache Tests
// ============================================================================

TEST(GlyphCache_construction) {
    GlyphCache cache(100);

    ASSERT(cache.getMaxEntries() == 100);
    ASSERT(cache.getEntryCount() == 0);
    ASSERT(cache.getHitCount() == 0);
    ASSERT(cache.getMissCount() == 0);
}

TEST(GlyphCache_settings) {
    GlyphCache cache;

    cache.setMaxEntries(500);
    ASSERT(cache.getMaxEntries() == 500);

    cache.resetStats();
    ASSERT(cache.getHitCount() == 0);
    ASSERT(cache.getMissCount() == 0);
}

TEST(GlyphCache_clear) {
    GlyphCache cache(100);

    cache.clear();
    ASSERT(cache.getEntryCount() == 0);
}

TEST(GlyphCache_frame_update) {
    GlyphCache cache;

    // newFrame should not crash
    cache.newFrame();
    cache.newFrame();
    cache.newFrame();
}

// ============================================================================
// TextShaper Tests
// ============================================================================

TEST(TextShaper_construction) {
    TextShaper shaper;
    // Should construct without error
}

// ============================================================================
// TextCursor Tests
// ============================================================================

TEST(TextCursor_construction) {
    TextCursor cursor;

    // Default state
    ASSERT(cursor.getPosition().charIndex == 0);
    ASSERT_NEAR(cursor.getPosition().x, 0.0f, 0.001f);
    ASSERT(!cursor.hasSelection());
}

TEST(TextCursor_blink_settings) {
    TextCursor cursor;

    // Default blink rate
    ASSERT_NEAR(cursor.getBlinkRate(), 0.5f, 0.001f);

    // Set blink rate
    cursor.setBlinkRate(0.25f);
    ASSERT_NEAR(cursor.getBlinkRate(), 0.25f, 0.001f);

    // Enable/disable blink
    cursor.setBlinkEnabled(false);
    ASSERT(cursor.isCursorVisible()); // Always visible when blink disabled
}

TEST(TextCursor_update) {
    TextCursor cursor;
    cursor.setBlinkEnabled(true);
    cursor.setBlinkRate(0.1f);

    // Initial state - visible
    ASSERT(cursor.isCursorVisible());

    // After half the blink period - should still be visible
    cursor.update(0.05f);
    ASSERT(cursor.isCursorVisible());

    // After full blink period - should toggle
    cursor.update(0.05f);
    ASSERT(!cursor.isCursorVisible());

    // After another period - toggle back
    cursor.update(0.1f);
    ASSERT(cursor.isCursorVisible());
}

TEST(TextCursor_selection) {
    TextCursor cursor;

    // Create mock shaped run
    ShapedRun run;
    for (int i = 0; i < 5; ++i) {
        ShapedGlyph g;
        g.glyphID = 65 + i; // A, B, C, D, E
        g.cluster = i;
        g.xAdvance = 500; // Font units
        run.glyphs.push_back(g);
    }

    cursor.init(run, "ABCDE", 16.0f, 12.0f);

    // Initial state
    ASSERT(cursor.getPosition().charIndex == 0);
    ASSERT(!cursor.hasSelection());

    // Select all
    cursor.selectAll();
    ASSERT(cursor.hasSelection());
    ASSERT(cursor.getSelection().start == 0);
    ASSERT(cursor.getSelection().end == 5);

    // Clear selection
    cursor.clearSelection();
    ASSERT(!cursor.hasSelection());

    // Manual selection
    cursor.setSelection(1, 3);
    ASSERT(cursor.hasSelection());
    ASSERT(cursor.getSelection().start == 1);
    ASSERT(cursor.getSelection().end == 3);
}

TEST(TextCursor_navigation) {
    TextCursor cursor;

    // Create mock shaped run
    ShapedRun run;
    for (int i = 0; i < 5; ++i) {
        ShapedGlyph g;
        g.glyphID = 65 + i;
        g.cluster = i;
        g.xAdvance = 500;
        run.glyphs.push_back(g);
    }

    cursor.init(run, "ABCDE", 16.0f, 12.0f);
    cursor.setPosition(2); // Position at index 2

    ASSERT(cursor.getPosition().charIndex == 2);

    // Move right
    cursor.moveRight();
    ASSERT(cursor.getPosition().charIndex == 3);

    // Move left
    cursor.moveLeft();
    ASSERT(cursor.getPosition().charIndex == 2);

    // Move to start
    cursor.moveToStart();
    ASSERT(cursor.getPosition().charIndex == 0);

    // Move to end
    cursor.moveToEnd();
    ASSERT(cursor.getPosition().charIndex == 5); // After last char
}

// ============================================================================
// VariableFont Tests
// ============================================================================

TEST(VariableFont_construction) {
    VariableFont varFont;

    // Not variable until loaded
    ASSERT(!varFont.isVariable());
    ASSERT(varFont.getAxes().empty());
    ASSERT(varFont.getInstances().empty());
}

TEST(VariableFont_axis_access) {
    VariableFont varFont;

    // Without loading, axis access should return nullptr
    const FontAxis* axis = varFont.getAxis("wght");
    ASSERT(axis == nullptr);

    // Default value should be 0 for non-existent axis
    float value = varFont.getAxisValue("wght");
    ASSERT_NEAR(value, 0.0f, 0.001f);
}

// ============================================================================
// OTFParser Tests
// ============================================================================

TEST(OTFParser_construction) {
    OTFParser parser;

    ASSERT(!parser.hasCFF());
    ASSERT(parser.getGlyphCount() == 0);
}

TEST(OTFParser_empty_parse) {
    OTFParser parser;

    // Empty data should fail
    std::vector<uint8_t> emptyData;
    bool result = parser.parseCFF(emptyData, 0, 0);
    ASSERT(!result);
}

// ============================================================================
// Main
// ============================================================================

int main() {
    printf("\n======== DaktLib-GUI Phase 2 Unit Tests ========\n\n");

    // Vec3 tests
    TestRunner_Vec3_operations runner_Vec3_operations;

    // BinaryStream tests
    TestRunner_BinaryStream_reading runner_BinaryStream_reading;
    TestRunner_BinaryStream_seeking runner_BinaryStream_seeking;

    // SDFGenerator tests
    TestRunner_SDFGenerator_basic runner_SDFGenerator_basic;
    TestRunner_SDFGenerator_empty_glyph runner_SDFGenerator_empty_glyph;
    TestRunner_SDFGenerator_shape_construction runner_SDFGenerator_shape_construction;

    // GlyphAtlas tests
    TestRunner_GlyphAtlas_construction runner_GlyphAtlas_construction;
    TestRunner_GlyphAtlas_clear runner_GlyphAtlas_clear;
    TestRunner_GlyphAtlas_settings runner_GlyphAtlas_settings;

    // GlyphCache tests
    TestRunner_GlyphCache_construction runner_GlyphCache_construction;
    TestRunner_GlyphCache_settings runner_GlyphCache_settings;
    TestRunner_GlyphCache_clear runner_GlyphCache_clear;
    TestRunner_GlyphCache_frame_update runner_GlyphCache_frame_update;

    // TextShaper tests
    TestRunner_TextShaper_construction runner_TextShaper_construction;

    // TextCursor tests
    TestRunner_TextCursor_construction runner_TextCursor_construction;
    TestRunner_TextCursor_blink_settings runner_TextCursor_blink_settings;
    TestRunner_TextCursor_update runner_TextCursor_update;
    TestRunner_TextCursor_selection runner_TextCursor_selection;
    TestRunner_TextCursor_navigation runner_TextCursor_navigation;

    // VariableFont tests
    TestRunner_VariableFont_construction runner_VariableFont_construction;
    TestRunner_VariableFont_axis_access runner_VariableFont_axis_access;

    // OTFParser tests
    TestRunner_OTFParser_construction runner_OTFParser_construction;
    TestRunner_OTFParser_empty_parse runner_OTFParser_empty_parse;

    printf("\n======== ✓ All Phase 2 tests passed! ========\n\n");
    return 0;
}
