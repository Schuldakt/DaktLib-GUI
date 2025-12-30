// ============================================================================
// DaktLib GUI Module - D3D11 Backend Implementation
// ============================================================================

#include <dakt/gui/D3D11Backend.hpp>

#ifdef _WIN32
    #include <d3d11.h>
    #include <d3dcompiler.h>

    #include <cstring>

    #pragma comment(lib, "d3d11.lib")
    #pragma comment(lib, "d3dcompiler.lib")

namespace dakt::gui
{

// ============================================================================
// Shader Source Code
// ============================================================================

static const char* g_vertexShaderSource = R"(
cbuffer ConstantBuffer : register(b0) {
    float4x4 ProjectionMatrix;
};

struct VS_INPUT {
    float2 pos : POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;
    output.pos = mul(ProjectionMatrix, float4(input.pos.xy, 0.0f, 1.0f));
    output.col = input.col;
    output.uv  = input.uv;
    return output;
}
)";

static const char* g_pixelShaderSource = R"(
struct PS_INPUT {
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

sampler sampler0 : register(s0);
Texture2D texture0 : register(t0);

float4 main(PS_INPUT input) : SV_Target {
    float4 texColor = texture0.Sample(sampler0, input.uv);
    return input.col * texColor;
}
)";

// ============================================================================
// Global Backend Instance
// ============================================================================

static D3D11Backend* g_backend = nullptr;

D3D11Backend* getD3D11Backend()
{
    return g_backend;
}

void setD3D11Backend(D3D11Backend* backend)
{
    g_backend = backend;
}

// ============================================================================
// D3D11Backend Implementation
// ============================================================================

D3D11Backend::D3D11Backend() = default;

D3D11Backend::~D3D11Backend()
{
    shutdown();
}

bool D3D11Backend::initialize(const D3D11BackendConfig& config)
{
    if (m_initialized)
    {
        return true;
    }

    if (!config.device || !config.deviceContext)
    {
        return false;
    }

    m_device = config.device;
    m_deviceContext = config.deviceContext;

    // Create shaders
    if (!createShaders())
    {
        shutdown();
        return false;
    }

    // Create render states
    if (!createRenderStates())
    {
        shutdown();
        return false;
    }

    // Create initial buffers
    if (!createBuffers(config.initialVertexBufferSize, config.initialIndexBufferSize))
    {
        shutdown();
        return false;
    }

    m_initialized = true;
    return true;
}

void D3D11Backend::shutdown()
{
    if (m_fontTextureId != 0)
    {
        destroyTexture(m_fontTextureId);
        m_fontTextureId = 0;
    }

    if (m_samplerState)
    {
        m_samplerState->Release();
        m_samplerState = nullptr;
    }
    if (m_depthStencilState)
    {
        m_depthStencilState->Release();
        m_depthStencilState = nullptr;
    }
    if (m_rasterizerState)
    {
        m_rasterizerState->Release();
        m_rasterizerState = nullptr;
    }
    if (m_blendState)
    {
        m_blendState->Release();
        m_blendState = nullptr;
    }
    if (m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = nullptr;
    }
    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = nullptr;
    }
    if (m_constantBuffer)
    {
        m_constantBuffer->Release();
        m_constantBuffer = nullptr;
    }
    if (m_inputLayout)
    {
        m_inputLayout->Release();
        m_inputLayout = nullptr;
    }
    if (m_pixelShader)
    {
        m_pixelShader->Release();
        m_pixelShader = nullptr;
    }
    if (m_vertexShader)
    {
        m_vertexShader->Release();
        m_vertexShader = nullptr;
    }

    m_device = nullptr;
    m_deviceContext = nullptr;
    m_vertexBufferSize = 0;
    m_indexBufferSize = 0;
    m_initialized = false;
}

bool D3D11Backend::createShaders()
{
    HRESULT hr;

    // Compile vertex shader
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    hr = D3DCompile(g_vertexShaderSource, strlen(g_vertexShaderSource), nullptr, nullptr, nullptr, "main", "vs_4_0",
                    D3DCOMPILE_ENABLE_STRICTNESS, 0, &vsBlob, &errorBlob);

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            errorBlob->Release();
        }
        return false;
    }

    hr = m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);

    if (FAILED(hr))
    {
        vsBlob->Release();
        return false;
    }

    // Create input layout
    D3D11_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(DrawVertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(DrawVertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(DrawVertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    hr = m_device->CreateInputLayout(inputLayout, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                                     &m_inputLayout);

    vsBlob->Release();

    if (FAILED(hr))
    {
        return false;
    }

    // Compile pixel shader
    ID3DBlob* psBlob = nullptr;

    hr = D3DCompile(g_pixelShaderSource, strlen(g_pixelShaderSource), nullptr, nullptr, nullptr, "main", "ps_4_0",
                    D3DCOMPILE_ENABLE_STRICTNESS, 0, &psBlob, &errorBlob);

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            errorBlob->Release();
        }
        return false;
    }

    hr = m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);

    psBlob->Release();

    if (FAILED(hr))
    {
        return false;
    }

    // Create constant buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(f32) * 16;  // 4x4 matrix
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = m_device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer);

    return SUCCEEDED(hr);
}

bool D3D11Backend::createRenderStates()
{
    HRESULT hr;

    // Create blend state (alpha blending)
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = m_device->CreateBlendState(&blendDesc, &m_blendState);
    if (FAILED(hr))
        return false;

    // Create rasterizer state (no culling, scissor test enabled)
    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.ScissorEnable = TRUE;
    rsDesc.DepthClipEnable = TRUE;

    hr = m_device->CreateRasterizerState(&rsDesc, &m_rasterizerState);
    if (FAILED(hr))
        return false;

    // Create depth stencil state (depth test disabled)
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = FALSE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    dsDesc.StencilEnable = FALSE;

    hr = m_device->CreateDepthStencilState(&dsDesc, &m_depthStencilState);
    if (FAILED(hr))
        return false;

    // Create sampler state (linear filtering)
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

    hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerState);
    if (FAILED(hr))
        return false;

    return true;
}

bool D3D11Backend::createBuffers(u32 vertexCount, u32 indexCount)
{
    return resizeVertexBuffer(vertexCount) && resizeIndexBuffer(indexCount);
}

bool D3D11Backend::resizeVertexBuffer(u32 newSize)
{
    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = nullptr;
    }

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = newSize * sizeof(DrawVertex);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_vertexBuffer);
    if (FAILED(hr))
        return false;

    m_vertexBufferSize = newSize;
    return true;
}

bool D3D11Backend::resizeIndexBuffer(u32 newSize)
{
    if (m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = nullptr;
    }

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = newSize * sizeof(u32);
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_indexBuffer);
    if (FAILED(hr))
        return false;

    m_indexBufferSize = newSize;
    return true;
}

void D3D11Backend::beginFrame(i32 displayWidth, i32 displayHeight)
{
    m_displayWidth = displayWidth;
    m_displayHeight = displayHeight;
}

void D3D11Backend::render(const DrawList& drawList)
{
    const auto& vertices = drawList.vertices();
    const auto& indices = drawList.indices();
    const auto& commands = drawList.commands();

    if (vertices.empty() || indices.empty() || commands.empty())
    {
        return;
    }

    // Resize buffers if needed
    if (vertices.size() > m_vertexBufferSize)
    {
        if (!resizeVertexBuffer(static_cast<u32>(vertices.size() * 1.5)))
        {
            return;
        }
    }

    if (indices.size() > m_indexBufferSize)
    {
        if (!resizeIndexBuffer(static_cast<u32>(indices.size() * 1.5)))
        {
            return;
        }
    }

    // Upload vertex data
    D3D11_MAPPED_SUBRESOURCE vtxMapped;
    if (FAILED(m_deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vtxMapped)))
    {
        return;
    }
    std::memcpy(vtxMapped.pData, vertices.data(), vertices.size() * sizeof(DrawVertex));
    m_deviceContext->Unmap(m_vertexBuffer, 0);

    // Upload index data
    D3D11_MAPPED_SUBRESOURCE idxMapped;
    if (FAILED(m_deviceContext->Map(m_indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &idxMapped)))
    {
        return;
    }
    std::memcpy(idxMapped.pData, indices.data(), indices.size() * sizeof(u32));
    m_deviceContext->Unmap(m_indexBuffer, 0);

    // Setup render state
    setupRenderState(m_displayWidth, m_displayHeight);

    // Render commands
    u32 globalIdxOffset = 0;
    u32 globalVtxOffset = 0;

    for (const auto& cmd : commands)
    {
        // Set clip rect as scissor
        D3D11_RECT scissor;
        scissor.left = static_cast<LONG>(cmd.clipRect.min.x);
        scissor.top = static_cast<LONG>(cmd.clipRect.min.y);
        scissor.right = static_cast<LONG>(cmd.clipRect.max.x);
        scissor.bottom = static_cast<LONG>(cmd.clipRect.max.y);
        m_deviceContext->RSSetScissorRects(1, &scissor);

        // Set texture
        ID3D11ShaderResourceView* srv = reinterpret_cast<ID3D11ShaderResourceView*>(cmd.textureId);
        if (srv)
        {
            m_deviceContext->PSSetShaderResources(0, 1, &srv);
        }

        // Draw
        m_deviceContext->DrawIndexed(cmd.indexCount, globalIdxOffset, globalVtxOffset);

        globalIdxOffset += cmd.indexCount;
    }

    // Restore render state
    restoreRenderState();
}

void D3D11Backend::endFrame()
{
    // Nothing to do for now
}

void D3D11Backend::setupRenderState(i32 displayWidth, i32 displayHeight)
{
    // Save current state
    m_deviceContext->RSGetState(&m_savedState.rasterizerState);
    m_deviceContext->OMGetBlendState(&m_savedState.blendState, m_savedState.blendFactor, &m_savedState.sampleMask);
    m_deviceContext->OMGetDepthStencilState(&m_savedState.depthStencilState, &m_savedState.stencilRef);
    m_deviceContext->PSGetShaderResources(0, 1, &m_savedState.psShaderResource);
    m_deviceContext->PSGetSamplers(0, 1, &m_savedState.psSampler);
    m_deviceContext->PSGetShader(&m_savedState.pixelShader, nullptr, nullptr);
    m_deviceContext->VSGetShader(&m_savedState.vertexShader, nullptr, nullptr);
    m_deviceContext->IAGetInputLayout(&m_savedState.inputLayout);
    m_deviceContext->IAGetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY*)&m_savedState.primitiveTopology);
    m_deviceContext->IAGetVertexBuffers(0, 1, &m_savedState.vertexBuffer, &m_savedState.vertexStride,
                                        &m_savedState.vertexOffset);
    m_deviceContext->IAGetIndexBuffer(&m_savedState.indexBuffer, (DXGI_FORMAT*)&m_savedState.indexFormat,
                                      &m_savedState.indexOffset);
    m_deviceContext->VSGetConstantBuffers(0, 1, &m_savedState.vsConstantBuffer);

    // Setup viewport
    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<f32>(displayWidth);
    viewport.Height = static_cast<f32>(displayHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_deviceContext->RSSetViewports(1, &viewport);

    // Setup shader and vertex layout
    m_deviceContext->IASetInputLayout(m_inputLayout);
    m_deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
    m_deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

    // Update projection matrix
    {
        f32 L = 0.0f;
        f32 R = static_cast<f32>(displayWidth);
        f32 T = 0.0f;
        f32 B = static_cast<f32>(displayHeight);
        f32 mvp[4][4] = {
            {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
            {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
            {0.0f, 0.0f, 0.5f, 0.0f},
            {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
        };

        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(m_deviceContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        {
            std::memcpy(mapped.pData, mvp, sizeof(mvp));
            m_deviceContext->Unmap(m_constantBuffer, 0);
        }
    }

    // Setup buffers
    u32 stride = sizeof(DrawVertex);
    u32 offset = 0;
    m_deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    m_deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_deviceContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
    m_deviceContext->PSSetSamplers(0, 1, &m_samplerState);

    // Setup render state
    f32 blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    m_deviceContext->OMSetBlendState(m_blendState, blendFactor, 0xFFFFFFFF);
    m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 0);
    m_deviceContext->RSSetState(m_rasterizerState);
}

void D3D11Backend::restoreRenderState()
{
    m_deviceContext->RSSetState(m_savedState.rasterizerState);
    if (m_savedState.rasterizerState)
        m_savedState.rasterizerState->Release();

    m_deviceContext->OMSetBlendState(m_savedState.blendState, m_savedState.blendFactor, m_savedState.sampleMask);
    if (m_savedState.blendState)
        m_savedState.blendState->Release();

    m_deviceContext->OMSetDepthStencilState(m_savedState.depthStencilState, m_savedState.stencilRef);
    if (m_savedState.depthStencilState)
        m_savedState.depthStencilState->Release();

    m_deviceContext->PSSetShaderResources(0, 1, &m_savedState.psShaderResource);
    if (m_savedState.psShaderResource)
        m_savedState.psShaderResource->Release();

    m_deviceContext->PSSetSamplers(0, 1, &m_savedState.psSampler);
    if (m_savedState.psSampler)
        m_savedState.psSampler->Release();

    m_deviceContext->PSSetShader(m_savedState.pixelShader, nullptr, 0);
    if (m_savedState.pixelShader)
        m_savedState.pixelShader->Release();

    m_deviceContext->VSSetShader(m_savedState.vertexShader, nullptr, 0);
    if (m_savedState.vertexShader)
        m_savedState.vertexShader->Release();

    m_deviceContext->IASetInputLayout(m_savedState.inputLayout);
    if (m_savedState.inputLayout)
        m_savedState.inputLayout->Release();

    m_deviceContext->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)m_savedState.primitiveTopology);

    m_deviceContext->IASetVertexBuffers(0, 1, &m_savedState.vertexBuffer, &m_savedState.vertexStride,
                                        &m_savedState.vertexOffset);
    if (m_savedState.vertexBuffer)
        m_savedState.vertexBuffer->Release();

    m_deviceContext->IASetIndexBuffer(m_savedState.indexBuffer, (DXGI_FORMAT)m_savedState.indexFormat,
                                      m_savedState.indexOffset);
    if (m_savedState.indexBuffer)
        m_savedState.indexBuffer->Release();

    m_deviceContext->VSSetConstantBuffers(0, 1, &m_savedState.vsConstantBuffer);
    if (m_savedState.vsConstantBuffer)
        m_savedState.vsConstantBuffer->Release();
}

uintptr_t D3D11Backend::createTexture(const byte* pixels, i32 width, i32 height, i32 channels)
{
    if (!pixels || width <= 0 || height <= 0)
    {
        return 0;
    }

    // Convert to RGBA if needed
    std::vector<byte> rgbaData;
    const byte* textureData = pixels;

    if (channels == 1)
    {
        rgbaData.resize(width * height * 4);
        for (i32 i = 0; i < width * height; ++i)
        {
            rgbaData[i * 4 + 0] = std::byte{255};
            rgbaData[i * 4 + 1] = std::byte{255};
            rgbaData[i * 4 + 2] = std::byte{255};
            rgbaData[i * 4 + 3] = pixels[i];
        }
        textureData = rgbaData.data();
    }
    else if (channels == 3)
    {
        rgbaData.resize(width * height * 4);
        for (i32 i = 0; i < width * height; ++i)
        {
            rgbaData[i * 4 + 0] = pixels[i * 3 + 0];
            rgbaData[i * 4 + 1] = pixels[i * 3 + 1];
            rgbaData[i * 4 + 2] = pixels[i * 3 + 2];
            rgbaData[i * 4 + 3] = std::byte{255};
        }
        textureData = rgbaData.data();
    }

    // Create texture
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = textureData;
    initData.SysMemPitch = width * 4;

    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = m_device->CreateTexture2D(&texDesc, &initData, &texture);
    if (FAILED(hr))
    {
        return 0;
    }

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    ID3D11ShaderResourceView* srv = nullptr;
    hr = m_device->CreateShaderResourceView(texture, &srvDesc, &srv);
    texture->Release();

    if (FAILED(hr))
    {
        return 0;
    }

    return reinterpret_cast<uintptr_t>(srv);
}

void D3D11Backend::updateTexture(uintptr_t textureId, const byte* pixels, i32 width, i32 height, i32 channels)
{
    (void)textureId;
    (void)pixels;
    (void)width;
    (void)height;
    (void)channels;
    // TODO: Implement texture update
}

void D3D11Backend::destroyTexture(uintptr_t textureId)
{
    if (textureId == 0)
        return;

    ID3D11ShaderResourceView* srv = reinterpret_cast<ID3D11ShaderResourceView*>(textureId);
    srv->Release();
}

uintptr_t D3D11Backend::createFontAtlasTexture(const byte* pixels, i32 width, i32 height)
{
    return createTexture(pixels, width, height, 1);
}

}  // namespace dakt::gui

#else  // !_WIN32

// Stub implementation for non-Windows platforms
namespace dakt::gui
{

static D3D11Backend* g_backend = nullptr;

D3D11Backend* getD3D11Backend()
{
    return g_backend;
}
void setD3D11Backend(D3D11Backend* backend)
{
    g_backend = backend;
}

D3D11Backend::D3D11Backend() = default;
D3D11Backend::~D3D11Backend() = default;

bool D3D11Backend::initialize(const D3D11BackendConfig&)
{
    return false;
}
void D3D11Backend::shutdown() {}
void D3D11Backend::beginFrame(i32, i32) {}
void D3D11Backend::render(const DrawList&) {}
void D3D11Backend::endFrame() {}
uintptr_t D3D11Backend::createTexture(const byte*, i32, i32, i32)
{
    return 0;
}
void D3D11Backend::updateTexture(uintptr_t, const byte*, i32, i32, i32) {}
void D3D11Backend::destroyTexture(uintptr_t) {}
uintptr_t D3D11Backend::createFontAtlasTexture(const byte*, i32, i32)
{
    return 0;
}
bool D3D11Backend::createShaders()
{
    return false;
}
bool D3D11Backend::createRenderStates()
{
    return false;
}
bool D3D11Backend::createBuffers(u32, u32)
{
    return false;
}
bool D3D11Backend::resizeVertexBuffer(u32)
{
    return false;
}
bool D3D11Backend::resizeIndexBuffer(u32)
{
    return false;
}
void D3D11Backend::setupRenderState(i32, i32) {}
void D3D11Backend::restoreRenderState() {}

}  // namespace dakt::gui

#endif  // _WIN32
