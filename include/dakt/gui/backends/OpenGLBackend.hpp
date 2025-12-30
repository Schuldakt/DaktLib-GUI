// ============================================================================
// DaktLib GUI Module - OpenGL Backend
// ============================================================================
// OpenGL 3.3+ rendering backend for the GUI system.
// Provides cross-platform rendering support using modern OpenGL with
// programmable shaders and vertex array objects.
// ============================================================================

#pragma once

#include <dakt/gui/backends/RenderBackend.hpp>

#include <unordered_map>

namespace dakt::gui
{

// ============================================================================
// OpenGL Backend Configuration
// ============================================================================

struct OpenGLBackendConfig
{
    // OpenGL context must be current when calling initialize()
    // The backend does not manage context creation

    // Initial buffer sizes (will grow as needed)
    u32 initialVertexBufferSize = 5000;
    u32 initialIndexBufferSize = 10000;

    // Use OpenGL ES (for embedded/mobile platforms)
    bool useGLES = false;

    // Minimum OpenGL version required
    i32 majorVersion = 3;
    i32 minorVersion = 3;

    // Enable debug output
    bool enableDebug = false;

    // Use DSA (Direct State Access) if available (OpenGL 4.5+)
    bool useDSA = false;

    // Custom loader function for OpenGL functions (optional)
    // If null, uses platform default (e.g., wglGetProcAddress, glXGetProcAddress)
    void* (*glGetProcAddress)(const char* name) = nullptr;
};

// ============================================================================
// OpenGL Backend Implementation
// ============================================================================

class OpenGLBackend final : public IRenderBackend
{
public:
    OpenGLBackend();
    ~OpenGLBackend() override;

    // Non-copyable
    OpenGLBackend(const OpenGLBackend&) = delete;
    OpenGLBackend& operator=(const OpenGLBackend&) = delete;

    // ========================================================================
    // Configuration
    // ========================================================================

    /// Configure the backend before initialization
    /// @param config Configuration options
    void configure(const OpenGLBackendConfig& config);

    // ========================================================================
    // IRenderBackend Implementation
    // ========================================================================

    [[nodiscard]] bool initialize() override;
    void shutdown() override;
    [[nodiscard]] bool isInitialized() const override { return m_initialized; }
    [[nodiscard]] BackendType getType() const override { return BackendType::OpenGL; }
    [[nodiscard]] const BackendCapabilities& getCapabilities() const override { return m_capabilities; }

    void beginFrame(i32 displayWidth, i32 displayHeight) override;
    void endFrame() override;

    void renderDrawList(const DrawList& drawList) override;
    void renderDrawLists(std::span<const DrawList*> drawLists) override;

    [[nodiscard]] TextureHandle createTexture(const TextureCreateInfo& info) override;
    void updateTexture(TextureHandle texture, const void* data, usize dataSize, u32 x = 0, u32 y = 0, u32 width = 0,
                       u32 height = 0) override;
    void destroyTexture(TextureHandle texture) override;

    void setViewport(i32 x, i32 y, i32 width, i32 height) override;
    void setScissorRect(i32 x, i32 y, i32 width, i32 height) override;

    void invalidateDeviceState() override;

    [[nodiscard]] void* getNativeDevice() const override { return nullptr; }   // OpenGL has no device
    [[nodiscard]] void* getNativeContext() const override { return nullptr; }  // Context is implicit

    // ========================================================================
    // OpenGL-Specific Methods
    // ========================================================================

    /// Get the shader program ID
    [[nodiscard]] u32 getShaderProgram() const { return m_shaderProgram; }

    /// Get the VAO ID
    [[nodiscard]] u32 getVAO() const { return m_vao; }

    /// Create a font texture from atlas data
    /// @param width Atlas width
    /// @param height Atlas height
    /// @param pixels Grayscale pixel data (R8)
    /// @return Handle to the created texture
    [[nodiscard]] TextureHandle createFontTexture(u32 width, u32 height, const u8* pixels);

    /// Check if an OpenGL extension is supported
    [[nodiscard]] bool isExtensionSupported(const char* extension) const;

    /// Get OpenGL version info
    [[nodiscard]] i32 getGLMajorVersion() const { return m_glMajorVersion; }
    [[nodiscard]] i32 getGLMinorVersion() const { return m_glMinorVersion; }
    [[nodiscard]] bool isGLES() const { return m_isGLES; }

private:
    // Initialization helpers
    bool loadGLFunctions();
    bool queryCapabilities();
    bool createShaders();
    bool createBuffers();
    void destroyDeviceObjects();

    // Buffer management
    void ensureVertexBufferSize(u32 requiredSize);
    void ensureIndexBufferSize(u32 requiredSize);

    // Rendering helpers
    void setupRenderState(i32 displayWidth, i32 displayHeight);
    void renderDrawCommands(const DrawList& drawList);
    void restoreGLState();

    // Shader compilation
    u32 compileShader(u32 type, const char* source);
    u32 linkProgram(u32 vertexShader, u32 fragmentShader);

    // OpenGL state backup/restore
    struct GLState
    {
        i32 lastProgram = 0;
        i32 lastTexture = 0;
        i32 lastSampler = 0;
        i32 lastArrayBuffer = 0;
        i32 lastVertexArray = 0;
        i32 lastElementArrayBuffer = 0;
        i32 lastBlendSrcRgb = 0;
        i32 lastBlendDstRgb = 0;
        i32 lastBlendSrcAlpha = 0;
        i32 lastBlendDstAlpha = 0;
        i32 lastBlendEquationRgb = 0;
        i32 lastBlendEquationAlpha = 0;
        i32 lastViewport[4] = {0};
        i32 lastScissorBox[4] = {0};
        bool lastEnableBlend = false;
        bool lastEnableCullFace = false;
        bool lastEnableDepthTest = false;
        bool lastEnableStencilTest = false;
        bool lastEnableScissorTest = false;
        bool lastEnablePrimitiveRestart = false;
    };

    void backupGLState(GLState& state);
    void restoreGLState(const GLState& state);

private:
    // Configuration
    OpenGLBackendConfig m_config{};
    BackendCapabilities m_capabilities{};
    bool m_initialized = false;

    // OpenGL version info
    i32 m_glMajorVersion = 0;
    i32 m_glMinorVersion = 0;
    bool m_isGLES = false;
    bool m_hasDSA = false;

    // Shader program
    u32 m_shaderProgram = 0;
    i32 m_attribLocationTex = 0;
    i32 m_attribLocationProjMtx = 0;
    i32 m_attribLocationVtxPos = 0;
    i32 m_attribLocationVtxUV = 0;
    i32 m_attribLocationVtxColor = 0;

    // Buffers
    u32 m_vbo = 0;
    u32 m_ebo = 0;
    u32 m_vao = 0;
    u32 m_vertexBufferSize = 0;
    u32 m_indexBufferSize = 0;

    // Font texture
    u32 m_fontTexture = 0;

    // Frame state
    i32 m_displayWidth = 0;
    i32 m_displayHeight = 0;

    // GL state backup
    GLState m_savedState{};

    // Texture tracking
    std::unordered_map<uintptr_t, u32> m_textures;
    uintptr_t m_nextTextureId = 1;
};

}  // namespace dakt::gui