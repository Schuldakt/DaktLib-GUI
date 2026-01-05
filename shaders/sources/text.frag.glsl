/**
 * @file text.frag.glsl
 * @brief Fragment shader for SDF/MSDF text rendering
 *
 * Renders text using Signed Distance Field techniques for
 * resolution-independent, high-quality text at any size.
 */

#version 450

// Inputs from vertex shader
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 fragColor;

// Uniforms
layout(binding = 0) uniform UniformBuffer {
    mat4 projection;
    float screenPxRange;  // Pixels covered by SDF spread
    float padding[3];
} ubo;

// SDF texture (R8 for SDF, RGB8 for MSDF)
layout(binding = 1) uniform sampler2D sdfTexture;

// Push constants for per-draw parameters
layout(push_constant) uniform PushConstants {
    int renderMode;  // 0 = SDF, 1 = MSDF
    float softness;  // Edge softness (0.0 = sharp, 1.0 = soft)
} pc;

// Output
layout(location = 0) out vec4 outColor;

// ============================================================================
// SDF Helper Functions
// ============================================================================

/**
 * Calculate median of three values (for MSDF)
 */
float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

/**
 * Calculate screen-space pixel distance for antialiasing
 */
float screenPxDistance(float sdfValue) {
    // Convert SDF value to screen pixels
    float screenPxRange = ubo.screenPxRange;
    if (screenPxRange < 1.0) screenPxRange = 1.0;
    
    // SDF is stored as 0.5 = edge, scale to signed distance
    float signedDist = (sdfValue - 0.5) * screenPxRange;
    return signedDist;
}

// ============================================================================
// Main
// ============================================================================

void main() {
    float opacity;
    
    if (pc.renderMode == 1) {
        // MSDF mode - use median of RGB channels
        vec3 msd = texture(sdfTexture, fragTexCoord).rgb;
        float sd = median(msd.r, msd.g, msd.b);
        float screenPxDist = screenPxDistance(sd);
        
        // Apply softness
        float edgeWidth = 0.5 + pc.softness * 0.5;
        opacity = clamp(screenPxDist / edgeWidth + 0.5, 0.0, 1.0);
    } else {
        // SDF mode - single channel
        float sd = texture(sdfTexture, fragTexCoord).r;
        float screenPxDist = screenPxDistance(sd);
        
        // Apply softness
        float edgeWidth = 0.5 + pc.softness * 0.5;
        opacity = clamp(screenPxDist / edgeWidth + 0.5, 0.0, 1.0);
    }
    
    // Discard fully transparent pixels
    if (opacity < 0.001) {
        discard;
    }
    
    // Output with text color and computed opacity
    outColor = vec4(fragColor.rgb, fragColor.a * opacity);
}
