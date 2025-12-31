// ============================================================================
// DaktLib GUI Module - D3D11 Backend Implementation
// ============================================================================

#include <dakt/gui/backends/D3D11Backend.hpp>

#ifdef _WIN32
    #include <d3d11.h>
    #include <d3dcompiler.h>

    #include <cstring>
    #include <vector>

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
// Saved D3D11 State
// ============================================================================

struct SavedD3D11State
{
    ID3D11RasterizerState* rasterizerState = nullptr;
    ID3D11BlendState* blendState = nullptr;
    f32 blendFactor[4] = {0};
    u32 sampleMask = 0;
    ID3D11DepthStencilState* depthStencilState = nullptr;
    u32 stencilRef = 0;
    ID3D11ShaderResourceView* psShaderResource = nullptr;
    ID3D11SamplerState* psSampler = nullptr;
    ID3D11PixelShader* pixelShader = nullptr;
    ID3D11VertexShader* vertexShader = nullptr;
    ID3D11InputLayout* inputLayout = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    ID3D11Buffer* vertexBuffer = nullptr;
    u32 vertexStride = 0;
    u32 vertexOffset = 0;
    ID3D11Buffer* indexBuffer = nullptr;
    DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN;
    u32 indexOffset = 0;
    ID3D11Buffer* vsConstantBuffer = nullptr;
};

// ============================================================================
// D3D11Backend Implementation
// ============================================================================

D3D11Backend::D3D11Backend() = default;

D3D11Backend::~D3D11Backend()
{
    shutdown();
}

void D3D11Backend::configure(const D3D11BackendConfig& config)
{
    m_config = config;
}

bool D3D11Backend::createDeviceObjects()
{
    if (!createShaders())
        return false;
    if (!createStates())
        return false;
    if (!createBuffers())
        return false;
    return true;
}

void D3D11Backend::destroyDeviceObjects()
{
    shutdown();
}

bool D3D11Backend::initialize()
{
    if (m_initialized)
    {
        return true;
    }

    if (!m_config.device || !m_config.deviceContext)
    {
        return false;
    }

    m_device = m_config.device;
    m_deviceContext = m_config.deviceContext;

    // Create shaders
    if (!createShaders())
    {
        shutdown();
        return false;
    }

    // Create render states
    if (!createStates())
    {
        shutdown();
        return false;
    }

    // Create initial buffers
    if (!createBuffers())
    {
        shutdown();
        return false;
    }

    // Create default white texture for solid color rendering
    {
        u8 whitePixel[4] = {255, 255, 255, 255};
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = 1;
        texDesc.Height = 1;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = whitePixel;
        initData.SysMemPitch = 4;

        ID3D11Texture2D* texture = nullptr;
        HRESULT hr = m_device->CreateTexture2D(&texDesc, &initData, &texture);
        if (SUCCEEDED(hr))
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = texDesc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            m_device->CreateShaderResourceView(texture, &srvDesc, &m_fontTextureView);
            texture->Release();
        }
    }

    // Setup capabilities
    m_capabilities.maxTextureSize = 16384;
    m_capabilities.maxTextures = 128;
    m_capabilities.maxVerticesPerBatch = 65536;
    m_capabilities.maxIndicesPerBatch = 65536 * 3;
    m_capabilities.supportsMSAA = true;

    m_initialized = true;
    return true;
}

void D3D11Backend::shutdown()
{
    if (m_fontTextureView)
    {
        m_fontTextureView->Release();
        m_fontTextureView = nullptr;
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

bool D3D11Backend::createStates()
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

bool D3D11Backend::createBuffers()
{
    ensureVertexBufferSize(m_config.initialVertexBufferSize);
    ensureIndexBufferSize(m_config.initialIndexBufferSize);
    return m_vertexBuffer != nullptr && m_indexBuffer != nullptr;
}

void D3D11Backend::ensureVertexBufferSize(u32 requiredSize)
{
    if (m_vertexBufferSize >= requiredSize)
        return;

    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = nullptr;
    }

    u32 newSize = std::max(requiredSize, m_vertexBufferSize * 2);
    if (newSize < 5000)
        newSize = 5000;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = newSize * sizeof(DrawVertex);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (SUCCEEDED(m_device->CreateBuffer(&bufferDesc, nullptr, &m_vertexBuffer)))
    {
        m_vertexBufferSize = newSize;
    }
}

void D3D11Backend::ensureIndexBufferSize(u32 requiredSize)
{
    if (m_indexBufferSize >= requiredSize)
        return;

    if (m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = nullptr;
    }

    u32 newSize = std::max(requiredSize, m_indexBufferSize * 2);
    if (newSize < 10000)
        newSize = 10000;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = newSize * sizeof(u32);
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (SUCCEEDED(m_device->CreateBuffer(&bufferDesc, nullptr, &m_indexBuffer)))
    {
        m_indexBufferSize = newSize;
    }
}

void D3D11Backend::beginFrame(i32 displayWidth, i32 displayHeight)
{
    m_displayWidth = displayWidth;
    m_displayHeight = displayHeight;
}

void D3D11Backend::endFrame()
{
    // Nothing to do
}

void D3D11Backend::renderDrawList(const DrawList& drawList)
{
    const auto& vertices = drawList.vertices();
    const auto& indices = drawList.indices();
    const auto& commands = drawList.commands();

    if (vertices.empty() || indices.empty() || commands.empty())
    {
        return;
    }

    // Resize buffers if needed
    ensureVertexBufferSize(static_cast<u32>(vertices.size()));
    ensureIndexBufferSize(static_cast<u32>(indices.size()));

    if (!m_vertexBuffer || !m_indexBuffer)
        return;

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
    renderDrawCommands(drawList);
}

void D3D11Backend::renderDrawLists(std::span<const DrawList*> drawLists)
{
    for (const auto* drawList : drawLists)
    {
        if (drawList)
        {
            renderDrawList(*drawList);
        }
    }
}

void D3D11Backend::setupRenderState(i32 displayWidth, i32 displayHeight)
{
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

void D3D11Backend::renderDrawCommands(const DrawList& drawList)
{
    const auto& commands = drawList.commands();

    u32 globalIdxOffset = 0;
    i32 globalVtxOffset = 0;

    for (const auto& cmd : commands)
    {
        // Set clip rect as scissor
        D3D11_RECT scissor;
        scissor.left = static_cast<LONG>(cmd.clipRect.min.x);
        scissor.top = static_cast<LONG>(cmd.clipRect.min.y);
        scissor.right = static_cast<LONG>(cmd.clipRect.max.x);
        scissor.bottom = static_cast<LONG>(cmd.clipRect.max.y);
        m_deviceContext->RSSetScissorRects(1, &scissor);

        // Set texture - use default white texture if none specified
        ID3D11ShaderResourceView* srv = reinterpret_cast<ID3D11ShaderResourceView*>(cmd.textureId);
        if (!srv)
        {
            srv = m_fontTextureView;  // Use default white texture
        }
        if (srv)
        {
            m_deviceContext->PSSetShaderResources(0, 1, &srv);
        }

        // Draw
        m_deviceContext->DrawIndexed(cmd.indexCount, globalIdxOffset, globalVtxOffset);

        globalIdxOffset += cmd.indexCount;
    }
}

TextureHandle D3D11Backend::createTexture(const TextureCreateInfo& info)
{
    TextureHandle handle{};

    if (info.width == 0 || info.height == 0)
        return handle;

    // Determine format
    DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    u32 pixelSize = 4;

    switch (info.format)
    {
        case TextureFormat::RGBA8:
            dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
            pixelSize = 4;
            break;
        case TextureFormat::BGRA8:
            dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
            pixelSize = 4;
            break;
        case TextureFormat::R8:
            dxgiFormat = DXGI_FORMAT_R8_UNORM;
            pixelSize = 1;
            break;
        case TextureFormat::RG8:
            dxgiFormat = DXGI_FORMAT_R8G8_UNORM;
            pixelSize = 2;
            break;
        default:
            return handle;
    }

    // Create texture
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = info.width;
    texDesc.Height = info.height;
    texDesc.MipLevels = info.generateMips ? 0 : 1;
    texDesc.ArraySize = 1;
    texDesc.Format = dxgiFormat;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    if (info.renderTarget)
        texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    if (info.generateMips)
        texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    D3D11_SUBRESOURCE_DATA initData = {};
    D3D11_SUBRESOURCE_DATA* pInitData = nullptr;

    if (info.initialData)
    {
        initData.pSysMem = info.initialData;
        initData.SysMemPitch = info.width * pixelSize;
        pInitData = &initData;
    }

    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = m_device->CreateTexture2D(&texDesc, pInitData, &texture);
    if (FAILED(hr))
    {
        return handle;
    }

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = dxgiFormat;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = info.generateMips ? static_cast<UINT>(-1) : 1;

    ID3D11ShaderResourceView* srv = nullptr;
    hr = m_device->CreateShaderResourceView(texture, &srvDesc, &srv);
    texture->Release();

    if (FAILED(hr))
    {
        return handle;
    }

    handle.id = reinterpret_cast<uintptr_t>(srv);
    handle.width = info.width;
    handle.height = info.height;
    handle.format = static_cast<u32>(info.format);

    return handle;
}

void D3D11Backend::updateTexture(TextureHandle texture, const void* data, usize dataSize, u32 x, u32 y, u32 width,
                                 u32 height)
{
    (void)dataSize;  // Not used directly - size is inferred from width/height

    if (!texture.isValid() || !data)
        return;

    ID3D11ShaderResourceView* srv = reinterpret_cast<ID3D11ShaderResourceView*>(texture.id);

    ID3D11Resource* resource = nullptr;
    srv->GetResource(&resource);

    if (!resource)
        return;

    u32 updateWidth = width > 0 ? width : texture.width;
    u32 updateHeight = height > 0 ? height : texture.height;

    D3D11_BOX box;
    box.left = x;
    box.top = y;
    box.front = 0;
    box.right = x + updateWidth;
    box.bottom = y + updateHeight;
    box.back = 1;

    // Determine row pitch based on format
    u32 pixelSize = 4;  // Assume RGBA8 by default
    if (texture.format == static_cast<u32>(TextureFormat::R8))
        pixelSize = 1;
    else if (texture.format == static_cast<u32>(TextureFormat::RG8))
        pixelSize = 2;

    m_deviceContext->UpdateSubresource(resource, 0, &box, data, updateWidth * pixelSize, 0);

    resource->Release();
}

void D3D11Backend::destroyTexture(TextureHandle texture)
{
    if (!texture.isValid())
        return;

    ID3D11ShaderResourceView* srv = reinterpret_cast<ID3D11ShaderResourceView*>(texture.id);
    srv->Release();
}

void D3D11Backend::setViewport(i32 x, i32 y, i32 width, i32 height)
{
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = static_cast<f32>(x);
    viewport.TopLeftY = static_cast<f32>(y);
    viewport.Width = static_cast<f32>(width);
    viewport.Height = static_cast<f32>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_deviceContext->RSSetViewports(1, &viewport);
}

void D3D11Backend::setScissorRect(i32 x, i32 y, i32 width, i32 height)
{
    D3D11_RECT rect;
    rect.left = static_cast<LONG>(x);
    rect.top = static_cast<LONG>(y);
    rect.right = static_cast<LONG>(x + width);
    rect.bottom = static_cast<LONG>(y + height);
    m_deviceContext->RSSetScissorRects(1, &rect);
}

void D3D11Backend::invalidateDeviceState()
{
    // Force re-setup of render state on next draw
}

TextureHandle D3D11Backend::createFontTexture(u32 width, u32 height, const u8* pixels)
{
    // Create RGBA texture from grayscale font data
    std::vector<u8> rgbaData(width * height * 4);
    for (u32 i = 0; i < width * height; ++i)
    {
        rgbaData[i * 4 + 0] = 255;
        rgbaData[i * 4 + 1] = 255;
        rgbaData[i * 4 + 2] = 255;
        rgbaData[i * 4 + 3] = pixels[i];
    }

    TextureCreateInfo info;
    info.width = width;
    info.height = height;
    info.format = TextureFormat::RGBA8;
    info.initialData = rgbaData.data();
    info.initialDataSize = rgbaData.size();

    return createTexture(info);
}

}  // namespace dakt::gui

#else  // !_WIN32

// Stub implementation for non-Windows platforms
namespace dakt::gui
{

D3D11Backend::D3D11Backend() = default;
D3D11Backend::~D3D11Backend() = default;

void D3D11Backend::configure(const D3D11BackendConfig&) {}
bool D3D11Backend::initialize()
{
    return false;
}
void D3D11Backend::shutdown() {}
void D3D11Backend::beginFrame(i32, i32) {}
void D3D11Backend::endFrame() {}
void D3D11Backend::renderDrawList(const DrawList&) {}
void D3D11Backend::renderDrawLists(std::span<const DrawList*>) {}
TextureHandle D3D11Backend::createTexture(const TextureCreateInfo&)
{
    return {};
}
void D3D11Backend::updateTexture(TextureHandle, const void*, usize, u32, u32, u32, u32) {}
void D3D11Backend::destroyTexture(TextureHandle) {}
void D3D11Backend::setViewport(i32, i32, i32, i32) {}
void D3D11Backend::setScissorRect(i32, i32, i32, i32) {}
void D3D11Backend::invalidateDeviceState() {}
TextureHandle D3D11Backend::createFontTexture(u32, u32, const u8*)
{
    return {};
}
bool D3D11Backend::createDeviceObjects()
{
    return false;
}
void D3D11Backend::destroyDeviceObjects() {}
bool D3D11Backend::createShaders()
{
    return false;
}
bool D3D11Backend::createBuffers()
{
    return false;
}
bool D3D11Backend::createStates()
{
    return false;
}
void D3D11Backend::ensureVertexBufferSize(u32) {}
void D3D11Backend::ensureIndexBufferSize(u32) {}
void D3D11Backend::setupRenderState(i32, i32) {}
void D3D11Backend::renderDrawCommands(const DrawList&) {}

}  // namespace dakt::gui

#endif  // _WIN32
