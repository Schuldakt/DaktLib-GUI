// ============================================================================
// DaktLib GUI Module - D3D11 Backend
// ============================================================================
// DirectX 11 rendering backend for the GUI system.
// Handles vertex buffer management, shader compilation, and draw call submission.
// ============================================================================

#pragma once

#include <dakt/gui/backends/RenderBackend.hpp>

// Forward declarations for D3D11 types
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11Buffer;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11BlendState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11SamplerState;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;

namespace dakt::gui
{

// ============================================================================
// D3D11 Backend Configuration
// ============================================================================

struct D3D11BackendConfig
{
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* deviceContext = nullptr;

    // Initial buffer sizes (will grow as needed)
    u32 initialVertexBufferSize = 5000;
    u32 initialIndexBufferSize = 10000;

    // Enable debug features
    bool enableDebug = false;
};

// ============================================================================
// D3D11 Backend Implementation
// ============================================================================

class D3D11Backend final : public IRenderBackend
{
public:
    D3D11Backend();
    ~D3D11Backend() override;

    // Non-copyable
    D3D11Backend(const D3D11Backend&) = delete;
    D3D11Backend& operator=(const D3D11Backend&) = delete;

    // ========================================================================
    // Configuration
    // ========================================================================

    /// Configure the backend before initialization
    /// @param config Configuration options
    void configure(const D3D11BackendConfig& config);

    // ========================================================================
    // IRenderBackend Implementation
    // ========================================================================

    [[nodiscard]] bool initialize() override;
    void shutdown() override;
    [[nodiscard]] bool isInitialized() const override { return m_initialized; }
    [[nodiscard]] BackendType getType() const override { return BackendType::D3D11; }
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

    [[nodiscard]] void* getNativeDevice() const override { return m_device; }
    [[nodiscard]] void* getNativeContext() const override { return m_deviceContext; }

    // ========================================================================
    // D3D11-Specific Methods
    // ========================================================================

    /// Get the D3D11 device
    [[nodiscard]] ID3D11Device* getDevice() const { return m_device; }

    /// Get the D3D11 device context
    [[nodiscard]] ID3D11DeviceContext* getDeviceContext() const { return m_deviceContext; }

    /// Create a font texture from atlas data
    /// @param width Atlas width
    /// @param height Atlas height
    /// @param pixels Grayscale pixel data (R8)
    /// @return Handle to the created texture
    [[nodiscard]] TextureHandle createFontTexture(u32 width, u32 height, const u8* pixels);

private:
    // Initialization helpers
    bool createDeviceObjects();
    void destroyDeviceObjects();
    bool createShaders();
    bool createBuffers();
    bool createStates();

    // Buffer management
    void ensureVertexBufferSize(u32 requiredSize);
    void ensureIndexBufferSize(u32 requiredSize);

    // Rendering helpers
    void setupRenderState(i32 displayWidth, i32 displayHeight);
    void renderDrawCommands(const DrawList& drawList);

private:
    // Configuration
    D3D11BackendConfig m_config{};
    BackendCapabilities m_capabilities{};
    bool m_initialized = false;

    // D3D11 Device
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_deviceContext = nullptr;

    // Shaders
    ID3D11VertexShader* m_vertexShader = nullptr;
    ID3D11PixelShader* m_pixelShader = nullptr;
    ID3D11InputLayout* m_inputLayout = nullptr;

    // Buffers
    ID3D11Buffer* m_vertexBuffer = nullptr;
    ID3D11Buffer* m_indexBuffer = nullptr;
    ID3D11Buffer* m_constantBuffer = nullptr;
    u32 m_vertexBufferSize = 0;
    u32 m_indexBufferSize = 0;

    // States
    ID3D11BlendState* m_blendState = nullptr;
    ID3D11RasterizerState* m_rasterizerState = nullptr;
    ID3D11DepthStencilState* m_depthStencilState = nullptr;
    ID3D11SamplerState* m_samplerState = nullptr;

    // Font texture
    ID3D11ShaderResourceView* m_fontTextureView = nullptr;

    // Frame state
    i32 m_displayWidth = 0;
    i32 m_displayHeight = 0;
};

}  // namespace dakt::gui
