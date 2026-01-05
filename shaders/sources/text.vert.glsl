/**
 * @file text.vert.glsl
 * @brief Vertex shader for SDF text rendering
 *
 * Same as UI vertex shader but outputs additional data for text rendering.
 */

#version 450

// Vertex attributes
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec4 inColor;

// Uniforms
layout(binding = 0) uniform UniformBuffer {
    mat4 projection;
    float screenPxRange;  // For SDF scaling
    float padding[3];
} ubo;

// Outputs to fragment shader
layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec4 fragColor;

void main() {
    gl_Position = ubo.projection * vec4(inPosition, 0.0, 1.0);
    fragTexCoord = inTexCoord;
    fragColor = inColor;
}
