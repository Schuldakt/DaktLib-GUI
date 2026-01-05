/**
 * @file ShaderBytecode.hpp
 * @brief Pre-compiled SPIR-V shader bytecode
 *
 * Contains embedded SPIR-V bytecode for UI and text shaders.
 * These are minimal placeholder shaders - real shaders would be
 * compiled from GLSL sources using glslc.
 */

#pragma once

#include <cstdint>

namespace dakt::gui::shaders {

// ============================================================================
// UI Vertex Shader (ui.vert.glsl compiled to SPIR-V)
// ============================================================================

// Minimal vertex shader that passes through position/UV/color
static const uint32_t UI_VERT_SPV[] = {
    // SPIR-V Header
    0x07230203, // Magic number
    0x00010000, // Version 1.0
    0x0008000b, // Generator
    0x0000002e, // Bound
    0x00000000, // Schema

    // OpCapability Shader
    0x00020011,
    0x00000001,
    // OpExtInstImport "GLSL.std.450"
    0x0006000b,
    0x00000001,
    0x4c534c47,
    0x6474732e,
    0x3035342e,
    0x00000000,
    // OpMemoryModel Logical GLSL450
    0x0003000e,
    0x00000000,
    0x00000001,
    // OpEntryPoint Vertex %main "main" %gl_Position %inPosition %outTexCoord %inTexCoord %outColor %inColor
    0x000f0003,
    0x00000000,
    0x00000004,
    0x6e69616d,
    0x00000000,
    0x0000000d,
    0x00000012,
    0x0000001c,
    0x0000001e,
    0x00000025,
    0x00000027,

    // Note: This is a placeholder - real compiled shader would have full bytecode
};

static const size_t UI_VERT_SPV_SIZE = sizeof(UI_VERT_SPV);

// ============================================================================
// UI Fragment Shader (ui.frag.glsl compiled to SPIR-V)
// ============================================================================

static const uint32_t UI_FRAG_SPV[] = {
    // SPIR-V Header
    0x07230203, // Magic number
    0x00010000, // Version 1.0
    0x0008000b, // Generator
    0x00000018, // Bound
    0x00000000, // Schema

    // OpCapability Shader
    0x00020011,
    0x00000001,
    // OpExtInstImport "GLSL.std.450"
    0x0006000b,
    0x00000001,
    0x4c534c47,
    0x6474732e,
    0x3035342e,
    0x00000000,
    // OpMemoryModel Logical GLSL450
    0x0003000e,
    0x00000000,
    0x00000001,
    // OpEntryPoint Fragment %main "main" %outColor %inTexCoord %inColor
    0x000a0003,
    0x00000004,
    0x00000004,
    0x6e69616d,
    0x00000000,
    0x00000009,
    0x0000000b,
    0x00000014,
    // OpExecutionMode %main OriginUpperLeft
    0x00030010,
    0x00000004,
    0x00000007,
};

static const size_t UI_FRAG_SPV_SIZE = sizeof(UI_FRAG_SPV);

// ============================================================================
// Text Vertex Shader (text.vert.glsl compiled to SPIR-V)
// ============================================================================

static const uint32_t TEXT_VERT_SPV[] = {
    // Same structure as UI vertex shader
    0x07230203, 0x00010000, 0x0008000b, 0x0000002e, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e,
    0x00000000, 0x00000001, 0x000f0003, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x0000000d, 0x00000012, 0x0000001c, 0x0000001e, 0x00000025, 0x00000027,
};

static const size_t TEXT_VERT_SPV_SIZE = sizeof(TEXT_VERT_SPV);

// ============================================================================
// Text Fragment Shader (text.frag.glsl compiled to SPIR-V)
// ============================================================================

static const uint32_t TEXT_FRAG_SPV[] = {
    // SDF text shader - placeholder
    0x07230203, 0x00010000, 0x0008000b, 0x00000028, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e,
    0x00000000, 0x00000001, 0x000a0003, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000b, 0x00000014, 0x00030010, 0x00000004, 0x00000007,
};

static const size_t TEXT_FRAG_SPV_SIZE = sizeof(TEXT_FRAG_SPV);

} // namespace dakt::gui::shaders
