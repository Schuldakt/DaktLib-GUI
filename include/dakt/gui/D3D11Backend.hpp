// ============================================================================
// DaktLib GUI Module - D3D11 Backend
// ============================================================================
// DirectX 11 rendering backend for the GUI system.
// Handles vertex buffer management, shader compilation, and draw call submission.
// ============================================================================

#pragma once

#include <dakt/core/Types.hpp>
#include <dakt/gui/DrawList.hpp>
#include <dakt/gui/Types.hpp>

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

using namespace dakt::core;

// ============================================================================
// Backend Configuration
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
// D3D11 Backend
// ============================================================================

class D3D11Backend
{
public:
    D3D11Backend();
    ~D3D11Backend();

    // Non-copyable
    D3D11Backend(const D3D11Backend&) = delete;
    D3D11Backend& operator=(const D3D11Backend&) = delete;

    // Initialize the backend with D3D11 device
    bool initialize(const D3D11BackendConfig& config);

    // Shutdown and release all resources
    void shutdown();

    // Check if initialized
    bool isInitialized() const { return m_initialized; }

    // Begin a new frame (call before any GUI rendering)
    void beginFrame(i32 displayWidth, i32 displayHeight);

    // Render draw data from a DrawList
    void render(const DrawList& drawList);

    // End frame (finalize any pending operations)
    void endFrame();

    // Texture management
    uintptr_t createTexture(const byte* pixels, i32 width, i32 height, i32 channels = 4);
    void updateTexture(uintptr_t textureId, const byte* pixels, i32 width, i32 height, i32 channels = 4);
    void destroyTexture(uintptr_t textureId);

    // Get font texture (for built-in font atlas)
    uintptr_t getFontTexture() const { return m_fontTextureId; }
    void setFontTexture(uintptr_t textureId) { m_fontTextureId = textureId; }

    // Create font atlas texture from raw pixel data
    uintptr_t createFontAtlasTexture(const byte* pixels, i32 width, i32 height);

    // Device getters (for external use)
    ID3D11Device* getDevice() const { return m_device; }
    ID3D11DeviceContext* getDeviceContext() const { return m_deviceContext; }

private:
    // Create shaders
    bool createShaders();

    // Create render states
    bool createRenderStates();

    // Create/resize buffers
    bool createBuffers(u32 vertexCount, u32 indexCount);
    bool resizeVertexBuffer(u32 newSize);
    bool resizeIndexBuffer(u32 newSize);

    // Setup render state for drawing
    void setupRenderState(i32 displayWidth, i32 displayHeight);

    // Restore render state after drawing
    void restoreRenderState();

private:
    bool m_initialized = false;

    // D3D11 device references (not owned)
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_deviceContext = nullptr;

    // Shaders
    ID3D11VertexShader* m_vertexShader = nullptr;
    ID3D11PixelShader* m_pixelShader = nullptr;
    ID3D11InputLayout* m_inputLayout = nullptr;

    // Constant buffer for projection matrix
    ID3D11Buffer* m_constantBuffer = nullptr;

    // Vertex and index buffers
    ID3D11Buffer* m_vertexBuffer = nullptr;
    ID3D11Buffer* m_indexBuffer = nullptr;
    u32 m_vertexBufferSize = 0;
    u32 m_indexBufferSize = 0;

    // Render states
    ID3D11BlendState* m_blendState = nullptr;
    ID3D11RasterizerState* m_rasterizerState = nullptr;
    ID3D11DepthStencilState* m_depthStencilState = nullptr;
    ID3D11SamplerState* m_samplerState = nullptr;

    // Font texture
    uintptr_t m_fontTextureId = 0;

    // Saved render state (for restoration)
    struct SavedState
    {
        ID3D11RasterizerState* rasterizerState = nullptr;
        ID3D11BlendState* blendState = nullptr;
        f32 blendFactor[4] = {0, 0, 0, 0};
        u32 sampleMask = 0;
        ID3D11DepthStencilState* depthStencilState = nullptr;
        u32 stencilRef = 0;
        ID3D11ShaderResourceView* psShaderResource = nullptr;
        ID3D11SamplerState* psSampler = nullptr;
        ID3D11PixelShader* pixelShader = nullptr;
        ID3D11VertexShader* vertexShader = nullptr;
        ID3D11InputLayout* inputLayout = nullptr;
        u32 primitiveTopology = 0;
        ID3D11Buffer* vertexBuffer = nullptr;
        u32 vertexStride = 0;
        u32 vertexOffset = 0;
        ID3D11Buffer* indexBuffer = nullptr;
        u32 indexFormat = 0;
        u32 indexOffset = 0;
        ID3D11Buffer* vsConstantBuffer = nullptr;
    };

    SavedState m_savedState;

    // Current frame dimensions
    i32 m_displayWidth = 0;
    i32 m_displayHeight = 0;
};

// ============================================================================
// Global Backend Access
// ============================================================================

// Get the global D3D11 backend instance
D3D11Backend* getD3D11Backend();

// Set the global D3D11 backend instance
void setD3D11Backend(D3D11Backend* backend);

}  // namespace dakt::gui
