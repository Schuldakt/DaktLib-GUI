/**
 * @file phase3_tests.cpp
 * @brief Phase 3 unit tests for DaktLib-GUI rendering backend
 *
 * Tests backend interface, draw batching, and shader structures.
 */

#include "dakt/gui/backend/IRenderBackend.hpp"
#include "dakt/gui/draw/DrawBatcher.hpp"
#include "dakt/gui/draw/DrawList.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>

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

#define ASSERT_EQ(a, b)                                                                                                                                                                                                                                  \
    do {                                                                                                                                                                                                                                                 \
        if ((a) != (b)) {                                                                                                                                                                                                                                \
            printf("FAILED: %s == %s at %s:%d\n", #a, #b, __FILE__, __LINE__);                                                                                                                                                                           \
            assert(false);                                                                                                                                                                                                                               \
        }                                                                                                                                                                                                                                                \
    } while (0)

// ============================================================================
// IRenderBackend Interface Tests
// ============================================================================

TEST(backend_handles) {
    // Test opaque handles
    ASSERT(InvalidBuffer == 0);
    ASSERT(InvalidTexture == 0);
    ASSERT(InvalidShader == 0);
    ASSERT(InvalidPipeline == 0);

    BufferHandle buf = 123;
    ASSERT(buf != InvalidBuffer);

    TextureHandle tex = 456;
    ASSERT(tex != InvalidTexture);
}

TEST(buffer_usage_flags) {
    // Test buffer usage flag combinations
    BufferUsage usage = BufferUsage::Vertex | BufferUsage::Index;
    ASSERT(usage & BufferUsage::Vertex);
    ASSERT(usage & BufferUsage::Index);
    ASSERT(!(usage & BufferUsage::Uniform));
}

TEST(texture_format) {
    // Test texture formats exist
    TextureFormat fmt = TextureFormat::RGBA8;
    ASSERT(fmt == TextureFormat::RGBA8);

    fmt = TextureFormat::R8;
    ASSERT(fmt == TextureFormat::R8);

    fmt = TextureFormat::Depth32F;
    ASSERT(fmt == TextureFormat::Depth32F);
}

TEST(texture_usage_flags) {
    TextureUsage usage = TextureUsage::Sampled | TextureUsage::RenderTarget;
    ASSERT(static_cast<uint32_t>(usage) & static_cast<uint32_t>(TextureUsage::Sampled));
    ASSERT(static_cast<uint32_t>(usage) & static_cast<uint32_t>(TextureUsage::RenderTarget));
}

TEST(buffer_desc) {
    BufferDesc desc;
    desc.size = 1024;
    desc.usage = BufferUsage::Vertex;
    desc.hostVisible = true;

    ASSERT_EQ(desc.size, 1024u);
    ASSERT(desc.usage & BufferUsage::Vertex);
    ASSERT(desc.hostVisible);
}

TEST(texture_desc) {
    TextureDesc desc;
    desc.width = 512;
    desc.height = 512;
    desc.format = TextureFormat::RGBA8;
    desc.mipLevels = 1;

    ASSERT_EQ(desc.width, 512u);
    ASSERT_EQ(desc.height, 512u);
    ASSERT(desc.format == TextureFormat::RGBA8);
    ASSERT_EQ(desc.mipLevels, 1u);
}

TEST(backend_capabilities) {
    BackendCapabilities caps;
    caps.maxTextureSize = 8192;
    caps.supportsCompute = true;
    caps.deviceName = "Test Device";

    ASSERT_EQ(caps.maxTextureSize, 8192u);
    ASSERT(caps.supportsCompute);
    ASSERT(caps.deviceName == "Test Device");
}

// ============================================================================
// DrawBatcher Tests
// ============================================================================

TEST(draw_batcher_construction) {
    DrawBatcher batcher;

    auto stats = batcher.getStats();
    ASSERT_EQ(stats.originalCommandCount, 0u);
    ASSERT_EQ(stats.batchedCommandCount, 0u);
}

TEST(draw_batcher_reset) {
    DrawBatcher batcher;
    batcher.reset();

    auto stats = batcher.getStats();
    ASSERT_EQ(stats.drawCalls, 0u);
}

TEST(draw_batcher_empty_drawlist) {
    DrawBatcher batcher;
    DrawList drawList;

    batcher.batchCommands(drawList);

    auto stats = batcher.getStats();
    ASSERT_EQ(stats.originalCommandCount, 0u);
    ASSERT_EQ(stats.batchedCommandCount, 0u);
}

TEST(draw_batcher_single_command) {
    DrawBatcher batcher;
    DrawList drawList;

    // Draw a simple rect
    drawList.drawRectFilled(Rect(0, 0, 100, 100), Color(1, 0, 0, 1));

    batcher.batchCommands(drawList);

    auto stats = batcher.getStats();
    ASSERT(stats.originalCommandCount >= 1);
    ASSERT(stats.batchedCommandCount >= 1);
}

TEST(draw_batcher_multiple_same_state) {
    DrawBatcher batcher;
    batcher.setMergeCommands(true);

    DrawList drawList;

    // Draw multiple rects with same state - should merge
    drawList.drawRectFilled(Rect(0, 0, 50, 50), Color(1, 0, 0, 1));
    drawList.drawRectFilled(Rect(60, 0, 50, 50), Color(1, 0, 0, 1));
    drawList.drawRectFilled(Rect(120, 0, 50, 50), Color(1, 0, 0, 1));

    batcher.batchCommands(drawList);

    auto stats = batcher.getStats();
    // Commands should be merged (or at least not more than original)
    ASSERT(stats.batchedCommandCount <= stats.originalCommandCount);
}

TEST(draw_batcher_texture_changes) {
    DrawBatcher batcher;
    DrawList drawList;

    // Draw with different textures
    drawList.setTexture(1);
    drawList.drawRectFilled(Rect(0, 0, 50, 50), Color(1, 1, 1, 1));

    drawList.setTexture(2);
    drawList.drawRectFilled(Rect(60, 0, 50, 50), Color(1, 1, 1, 1));

    drawList.setTexture(1);
    drawList.drawRectFilled(Rect(120, 0, 50, 50), Color(1, 1, 1, 1));

    batcher.batchCommands(drawList);

    auto stats = batcher.getStats();
    ASSERT(stats.textureChanges >= 2); // At least 2 texture changes
}

TEST(draw_batcher_clip_rect) {
    DrawBatcher batcher;
    DrawList drawList;

    // Draw with different clip rects
    drawList.pushClipRect(Rect(0, 0, 100, 100));
    drawList.drawRectFilled(Rect(0, 0, 50, 50), Color(1, 0, 0, 1));
    drawList.popClipRect();

    drawList.pushClipRect(Rect(100, 0, 100, 100));
    drawList.drawRectFilled(Rect(100, 0, 50, 50), Color(0, 1, 0, 1));
    drawList.popClipRect();

    batcher.batchCommands(drawList);

    auto stats = batcher.getStats();
    ASSERT(stats.clipRectChanges >= 1);
}

TEST(draw_batcher_sort_by_texture) {
    DrawBatcher batcher;
    batcher.setSortByTexture(true);

    DrawList drawList;

    // Draw with interleaved textures
    drawList.setTexture(1);
    drawList.drawRectFilled(Rect(0, 0, 50, 50), Color(1, 1, 1, 1));

    drawList.setTexture(2);
    drawList.drawRectFilled(Rect(60, 0, 50, 50), Color(1, 1, 1, 1));

    drawList.setTexture(1);
    drawList.drawRectFilled(Rect(120, 0, 50, 50), Color(1, 1, 1, 1));

    batcher.batchCommands(drawList);

    // Sorting should group same textures together
    auto& commands = batcher.getBatchedCommands();
    ASSERT(!commands.empty());
}

// ============================================================================
// RenderState Tests
// ============================================================================

TEST(render_state_equality) {
    RenderState a;
    a.textureID = 1;
    a.clipRect = Rect(0, 0, 100, 100);
    a.isTextured = true;

    RenderState b;
    b.textureID = 1;
    b.clipRect = Rect(0, 0, 100, 100);
    b.isTextured = true;

    ASSERT(a == b);

    b.textureID = 2;
    ASSERT(a != b);
}

// ============================================================================
// DrawList Integration Tests
// ============================================================================

TEST(drawlist_vertices) {
    DrawList drawList;
    drawList.drawRectFilled(Rect(0, 0, 100, 100), Color(1, 0, 0, 1));

    auto& vertices = drawList.getVertices();
    ASSERT(!vertices.empty());
    ASSERT(vertices.size() >= 4); // At least 4 vertices for a rect
}

TEST(drawlist_indices) {
    DrawList drawList;
    drawList.drawRectFilled(Rect(0, 0, 100, 100), Color(1, 0, 0, 1));

    auto& indices = drawList.getIndices();
    ASSERT(!indices.empty());
    ASSERT(indices.size() >= 6); // At least 6 indices for 2 triangles
}

TEST(drawlist_commands) {
    DrawList drawList;
    drawList.drawRectFilled(Rect(0, 0, 100, 100), Color(1, 0, 0, 1));

    auto& commands = drawList.getCommands();
    ASSERT(!commands.empty());
}

TEST(drawlist_reset) {
    DrawList drawList;
    drawList.drawRectFilled(Rect(0, 0, 100, 100), Color(1, 0, 0, 1));

    ASSERT(!drawList.getVertices().empty());

    drawList.reset();

    ASSERT(drawList.getVertices().empty());
    ASSERT(drawList.getIndices().empty());
}

// ============================================================================
// Vertex Tests
// ============================================================================

TEST(vertex_construction) {
    Vertex v(Vec2(10, 20), Vec2(0.5f, 0.5f), Color(1, 0, 0, 1));

    ASSERT(v.position.x == 10.0f);
    ASSERT(v.position.y == 20.0f);
    ASSERT(v.uv.x == 0.5f);
    ASSERT(v.uv.y == 0.5f);
    ASSERT(v.color.r == 1.0f);
}

// ============================================================================
// Main
// ============================================================================

int main() {
    printf("\n======== DaktLib-GUI Phase 3 Unit Tests ========\n\n");

    // Backend interface tests
    TestRunner_backend_handles runner_backend_handles;
    TestRunner_buffer_usage_flags runner_buffer_usage_flags;
    TestRunner_texture_format runner_texture_format;
    TestRunner_texture_usage_flags runner_texture_usage_flags;
    TestRunner_buffer_desc runner_buffer_desc;
    TestRunner_texture_desc runner_texture_desc;
    TestRunner_backend_capabilities runner_backend_capabilities;

    // DrawBatcher tests
    TestRunner_draw_batcher_construction runner_draw_batcher_construction;
    TestRunner_draw_batcher_reset runner_draw_batcher_reset;
    TestRunner_draw_batcher_empty_drawlist runner_draw_batcher_empty_drawlist;
    TestRunner_draw_batcher_single_command runner_draw_batcher_single_command;
    TestRunner_draw_batcher_multiple_same_state runner_draw_batcher_multiple_same_state;
    TestRunner_draw_batcher_texture_changes runner_draw_batcher_texture_changes;
    TestRunner_draw_batcher_clip_rect runner_draw_batcher_clip_rect;
    TestRunner_draw_batcher_sort_by_texture runner_draw_batcher_sort_by_texture;

    // RenderState tests
    TestRunner_render_state_equality runner_render_state_equality;

    // DrawList integration tests
    TestRunner_drawlist_vertices runner_drawlist_vertices;
    TestRunner_drawlist_indices runner_drawlist_indices;
    TestRunner_drawlist_commands runner_drawlist_commands;
    TestRunner_drawlist_reset runner_drawlist_reset;

    // Vertex tests
    TestRunner_vertex_construction runner_vertex_construction;

    printf("\n======== ✓ All Phase 3 tests passed! ========\n\n");
    return 0;
}
