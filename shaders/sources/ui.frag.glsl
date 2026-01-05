/**
 * @file ui.frag.glsl
 * @brief Fragment shader for UI geometry rendering
 *
 * Supports both colored geometry and textured quads.
 */

#version 450

// Inputs from vertex shader
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 fragColor;

// Texture sampler
layout(binding = 1) uniform sampler2D texSampler;

// Output
layout(location = 0) out vec4 outColor;

void main() {
    vec4 texColor = texture(texSampler, fragTexCoord);
    outColor = fragColor * texColor;
}
