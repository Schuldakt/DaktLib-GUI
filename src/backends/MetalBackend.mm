// ============================================================================
// DaktLib GUI Module - Metal Backend Implementation
// ============================================================================

#include <dakt/gui/backends/MetalBackend.hpp>

#if defined(__APPLE__)

#import <Metal/Metal.h>
#import <simd/simd.h>
#include <dispatch/dispatch.h>
#include <cstring>

namespace dakt::gui
{

// ============================================================================
// Shader Source
// ============================================================================

namespace
{

const char* g_metalShaderSource = R"(
#include <metal_stdlib>
using namespace metal;

struct Uniforms {
    float4x4 projectionMatrix;
};

struct VertexIn {
    float2 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
    uchar4 color    [[attribute(2)]];
};

struct VertexOut {
    float4 position [[position]];
    float2 texCoord;
    float4 color;
};

vertex VertexOut vertex_main(VertexIn in [[stage_in]],
                             constant Uniforms& uniforms [[buffer(1)]]) {
    VertexOut out;
    out.position = uniforms.projectionMatrix * float4(in.position, 0, 1);
    out.texCoord = in.texCoord;
    out.color = float4(in.color) / 255.0;
    return out;
}

fragment float4 fragment_main(VertexOut in [[stage_in]],
                              texture2d<float> texture [[texture(0)]],
                              sampler textureSampler [[sampler(0)]]) {
    return in.color * texture.sample(textureSampler, in.texCoord);
}
)";

// Uniform buffer structure
struct MetalUniforms
{
    simd_float4x4 projectionMatrix;
};

}  // anonymous namespace

// ============================================================================
// MetalBackend Implementation
// ============================================================================

MetalBackend::MetalBackend() = default;

MetalBackend::~MetalBackend()
{
    shutdown();
}

void MetalBackend::configure(const MetalBackendConfig& config)
{
    m_config = config;
}

bool MetalBackend::initialize()
{
    if (m_initialized)
        return true;

    // Create or use provided device
    if (!createDevice())
        return false;

    // Create or use provided command queue
    if (!createCommandQueue())
        return false;

    // Query capabilities
    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device;
    
    m_capabilities.supportsMultiViewport = false;  // Metal has limited viewport support
    m_capabilities.supportsTextureArrays = true;
    m_capabilities.supportsMSAA = true;
    m_capabilities.supportsCompute = true;
    m_capabilities.supportsBindless = [device supportsFamily:MTLGPUFamilyApple6];
    m_capabilities.maxTextureSize = 16384;
    m_capabilities.maxTextures = 128;

    // Create pipeline state
    if (!createPipelineState())
    {
        shutdown();
        return false;
    }

    // Create depth stencil state
    if (!createDepthStencilState())
    {
        shutdown();
        return false;
    }

    // Create sampler state
    if (!createSamplerState())
    {
        shutdown();
        return false;
    }

    // Create frame resources
    if (!createBuffers())
    {
        shutdown();
        return false;
    }

    // Create frame semaphore for triple buffering
    m_frameSemaphore = dispatch_semaphore_create(m_config.numFramesInFlight);

    m_initialized = true;
    return true;
}

void MetalBackend::shutdown()
{
    if (!m_initialized)
        return;

    // Wait for all frames to complete
    for (u32 i = 0; i < m_config.numFramesInFlight; ++i)
    {
        dispatch_semaphore_wait((dispatch_semaphore_t)m_frameSemaphore, DISPATCH_TIME_FOREVER);
    }

    destroyDeviceObjects();

    m_device = nullptr;
    m_commandQueue = nullptr;
    m_initialized = false;
}

bool MetalBackend::createDevice()
{
    if (m_config.device)
    {
        m_device = m_config.device;
        m_ownsDevice = false;
    }
    else
    {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (!device)
            return false;
        
        m_device = (__bridge_retained void*)device;
        m_ownsDevice = true;
    }
    
    return m_device != nullptr;
}

bool MetalBackend::createCommandQueue()
{
    if (m_config.commandQueue)
    {
        m_commandQueue = m_config.commandQueue;
        m_ownsCommandQueue = false;
    }
    else
    {
        id<MTLDevice> device = (__bridge id<MTLDevice>)m_device;
        id<MTLCommandQueue> queue = [device newCommandQueue];
        if (!queue)
            return false;
        
        m_commandQueue = (__bridge_retained void*)queue;
        m_ownsCommandQueue = true;
    }
    
    return m_commandQueue != nullptr;
}

bool MetalBackend::createPipelineState()
{
    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device;
    NSError* error = nil;

    // Compile shaders
    NSString* shaderSource = [NSString stringWithUTF8String:g_metalShaderSource];
    MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
    options.fastMathEnabled = YES;

    id<MTLLibrary> library = [device newLibraryWithSource:shaderSource options:options error:&error];
    if (!library)
    {
        NSLog(@"Failed to compile Metal shaders: %@", error);
        return false;
    }

    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragment_main"];

    if (!vertexFunction || !fragmentFunction)
    {
        NSLog(@"Failed to find shader functions");
        return false;
    }

    // Create vertex descriptor
    MTLVertexDescriptor* vertexDescriptor = [[MTLVertexDescriptor alloc] init];

    // Position
    vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[0].offset = offsetof(DrawVertex, pos);
    vertexDescriptor.attributes[0].bufferIndex = 0;

    // Texture coordinates
    vertexDescriptor.attributes[1].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[1].offset = offsetof(DrawVertex, uv);
    vertexDescriptor.attributes[1].bufferIndex = 0;

    // Color
    vertexDescriptor.attributes[2].format = MTLVertexFormatUChar4;
    vertexDescriptor.attributes[2].offset = offsetof(DrawVertex, col);
    vertexDescriptor.attributes[2].bufferIndex = 0;

    vertexDescriptor.layouts[0].stride = sizeof(DrawVertex);
    vertexDescriptor.layouts[0].stepRate = 1;
    vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

    // Create pipeline descriptor
    MTLRenderPipelineDescriptor* pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDescriptor.label = @"DaktGUI Pipeline";
    pipelineDescriptor.vertexFunction = vertexFunction;
    pipelineDescriptor.fragmentFunction = fragmentFunction;
    pipelineDescriptor.vertexDescriptor = vertexDescriptor;
    pipelineDescriptor.rasterSampleCount = m_config.sampleCount;

    // Color attachment
    pipelineDescriptor.colorAttachments[0].pixelFormat = (MTLPixelFormat)m_config.colorPixelFormat;
    pipelineDescriptor.colorAttachments[0].blendingEnabled = YES;
    pipelineDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    pipelineDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    pipelineDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    pipelineDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
    pipelineDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    pipelineDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

    // Depth/stencil attachments
    if (m_config.depthPixelFormat != 0)
        pipelineDescriptor.depthAttachmentPixelFormat = (MTLPixelFormat)m_config.depthPixelFormat;
    if (m_config.stencilPixelFormat != 0)
        pipelineDescriptor.stencilAttachmentPixelFormat = (MTLPixelFormat)m_config.stencilPixelFormat;

    // Create pipeline state
    id<MTLRenderPipelineState> pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor
                                                                                      error:&error];
    if (!pipelineState)
    {
        NSLog(@"Failed to create pipeline state: %@", error);
        return false;
    }

    m_pipelineState = (__bridge_retained void*)pipelineState;
    return true;
}

bool MetalBackend::createDepthStencilState()
{
    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device;

    MTLDepthStencilDescriptor* descriptor = [[MTLDepthStencilDescriptor alloc] init];
    descriptor.depthCompareFunction = MTLCompareFunctionAlways;
    descriptor.depthWriteEnabled = NO;

    id<MTLDepthStencilState> state = [device newDepthStencilStateWithDescriptor:descriptor];
    if (!state)
        return false;

    m_depthStencilState = (__bridge_retained void*)state;
    return true;
}

bool MetalBackend::createSamplerState()
{
    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device;

    MTLSamplerDescriptor* descriptor = [[MTLSamplerDescriptor alloc] init];
    descriptor.minFilter = MTLSamplerMinMagFilterLinear;
    descriptor.magFilter = MTLSamplerMinMagFilterLinear;
    descriptor.mipFilter = MTLSamplerMipFilterNotMipmapped;
    descriptor.sAddressMode = MTLSamplerAddressModeRepeat;
    descriptor.tAddressMode = MTLSamplerAddressModeRepeat;
    descriptor.rAddressMode = MTLSamplerAddressModeRepeat;

    id<MTLSamplerState> state = [device newSamplerStateWithDescriptor:descriptor];
    if (!state)
        return false;

    m_samplerState = (__bridge_retained void*)state;
    return true;
}

bool MetalBackend::createBuffers()
{
    m_frameResources.resize(m_config.numFramesInFlight);
    
    // Buffers will be created on-demand when needed
    return true;
}

void MetalBackend::destroyDeviceObjects()
{
    // Release frame resources
    for (auto& frame : m_frameResources)
    {
        if (frame.vertexBuffer)
        {
            CFRelease(frame.vertexBuffer);
            frame.vertexBuffer = nullptr;
        }
        if (frame.indexBuffer)
        {
            CFRelease(frame.indexBuffer);
            frame.indexBuffer = nullptr;
        }
    }
    m_frameResources.clear();

    // Release font texture
    if (m_fontTexture.texture)
    {
        CFRelease(m_fontTexture.texture);
        m_fontTexture.texture = nullptr;
    }

    // Release tracked textures
    for (auto& tex : m_textures)
    {
        if (tex.texture)
        {
            CFRelease(tex.texture);
            tex.texture = nullptr;
        }
    }
    m_textures.clear();

    // Release states
    if (m_samplerState)
    {
        CFRelease(m_samplerState);
        m_samplerState = nullptr;
    }

    if (m_depthStencilState)
    {
        CFRelease(m_depthStencilState);
        m_depthStencilState = nullptr;
    }

    if (m_pipelineState)
    {
        CFRelease(m_pipelineState);
        m_pipelineState = nullptr;
    }

    // Release command queue if we own it
    if (m_ownsCommandQueue && m_commandQueue)
    {
        CFRelease(m_commandQueue);
        m_commandQueue = nullptr;
    }

    // Release device if we own it
    if (m_ownsDevice && m_device)
    {
        CFRelease(m_device);
        m_device = nullptr;
    }

    // Release semaphore
    if (m_frameSemaphore)
    {
        // Signal remaining waits
        for (u32 i = 0; i < m_config.numFramesInFlight; ++i)
        {
            dispatch_semaphore_signal((dispatch_semaphore_t)m_frameSemaphore);
        }
        m_frameSemaphore = nullptr;
    }
}

void MetalBackend::setRenderPassDescriptor(void* descriptor)
{
    m_renderPassDescriptor = descriptor;
}

void MetalBackend::setDrawable(void* drawable)
{
    m_currentDrawable = drawable;
}

void MetalBackend::beginFrame(i32 displayWidth, i32 displayHeight)
{
    m_displayWidth = displayWidth;
    m_displayHeight = displayHeight;

    // Wait for frame semaphore
    dispatch_semaphore_wait((dispatch_semaphore_t)m_frameSemaphore, DISPATCH_TIME_FOREVER);

    // Advance frame index
    m_frameIndex = (m_frameIndex + 1) % static_cast<u32>(m_frameResources.size());

    // Create command buffer
    id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)m_commandQueue;
    id<MTLCommandBuffer> commandBuffer = [queue commandBuffer];
    commandBuffer.label = @"DaktGUI Command Buffer";

    m_currentCommandBuffer = (__bridge_retained void*)commandBuffer;

    // Create render encoder if we have a render pass descriptor
    if (m_renderPassDescriptor)
    {
        MTLRenderPassDescriptor* rpd = (__bridge MTLRenderPassDescriptor*)m_renderPassDescriptor;
        id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:rpd];
        encoder.label = @"DaktGUI Render Encoder";

        m_currentEncoder = (__bridge_retained void*)encoder;
    }
}

void MetalBackend::endFrame()
{
    // End render encoder
    if (m_currentEncoder)
    {
        id<MTLRenderCommandEncoder> encoder = (__bridge_transfer id<MTLRenderCommandEncoder>)m_currentEncoder;
        [encoder endEncoding];
        m_currentEncoder = nullptr;
    }

    // Present drawable if available
    if (m_currentDrawable)
    {
        id<MTLCommandBuffer> commandBuffer = (__bridge id<MTLCommandBuffer>)m_currentCommandBuffer;
        id<MTLDrawable> drawable = (__bridge id<MTLDrawable>)m_currentDrawable;
        [commandBuffer presentDrawable:drawable];
    }

    // Add completion handler for semaphore
    id<MTLCommandBuffer> commandBuffer = (__bridge_transfer id<MTLCommandBuffer>)m_currentCommandBuffer;
    dispatch_semaphore_t semaphore = (dispatch_semaphore_t)m_frameSemaphore;
    
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
        dispatch_semaphore_signal(semaphore);
    }];

    // Commit
    [commandBuffer commit];

    m_currentCommandBuffer = nullptr;
    m_currentDrawable = nullptr;
}

void MetalBackend::renderDrawList(const DrawList& drawList)
{
    renderDrawCommands(drawList);
}

void MetalBackend::renderDrawLists(std::span<const DrawList*> drawLists)
{
    for (const auto* drawList : drawLists)
    {
        if (drawList)
            renderDrawCommands(*drawList);
    }
}

void MetalBackend::renderDrawCommands(const DrawList& drawList)
{
    if (!m_currentEncoder)
        return;

    const auto& vertices = drawList.getVertices();
    const auto& indices = drawList.getIndices();
    const auto& commands = drawList.getCommands();

    if (vertices.empty() || indices.empty() || commands.empty())
        return;

    auto& frame = getCurrentFrameResources();
    id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)m_currentEncoder;

    // Ensure buffer sizes
    u32 vertexSize = static_cast<u32>(vertices.size() * sizeof(DrawVertex));
    u32 indexSize = static_cast<u32>(indices.size() * sizeof(DrawIndex));

    ensureVertexBufferSize(vertexSize);
    ensureIndexBufferSize(indexSize);

    // Upload vertex data
    id<MTLBuffer> vertexBuffer = (__bridge id<MTLBuffer>)frame.vertexBuffer;
    std::memcpy([vertexBuffer contents], vertices.data(), vertexSize);

    // Upload index data
    id<MTLBuffer> indexBuffer = (__bridge id<MTLBuffer>)frame.indexBuffer;
    std::memcpy([indexBuffer contents], indices.data(), indexSize);

    // Set up render state
    setupRenderState();

    // Bind vertex buffer
    [encoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];

    // Set up projection matrix
    float L = 0.0f;
    float R = static_cast<float>(m_displayWidth);
    float T = 0.0f;
    float B = static_cast<float>(m_displayHeight);

    MetalUniforms uniforms;
    uniforms.projectionMatrix = simd_matrix(
        simd_make_float4(2.0f / (R - L), 0.0f, 0.0f, 0.0f),
        simd_make_float4(0.0f, 2.0f / (T - B), 0.0f, 0.0f),
        simd_make_float4(0.0f, 0.0f, -1.0f, 0.0f),
        simd_make_float4((R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f)
    );

    [encoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:1];

    // Process draw commands
    u32 indexOffset = 0;

    for (const auto& cmd : commands)
    {
        // Set scissor rect
        MTLScissorRect scissor;
        scissor.x = static_cast<NSUInteger>(std::max(0.0f, cmd.clipRect.min.x));
        scissor.y = static_cast<NSUInteger>(std::max(0.0f, cmd.clipRect.min.y));
        scissor.width = static_cast<NSUInteger>(cmd.clipRect.max.x - cmd.clipRect.min.x);
        scissor.height = static_cast<NSUInteger>(cmd.clipRect.max.y - cmd.clipRect.min.y);
        
        // Clamp to viewport
        scissor.width = std::min(scissor.width, static_cast<NSUInteger>(m_displayWidth) - scissor.x);
        scissor.height = std::min(scissor.height, static_cast<NSUInteger>(m_displayHeight) - scissor.y);
        
        if (scissor.width > 0 && scissor.height > 0)
        {
            [encoder setScissorRect:scissor];
        }

        // Bind texture
        id<MTLTexture> texture = nil;
        if (cmd.textureId != 0)
        {
            // Look up texture
            uintptr_t texId = cmd.textureId;
            if (texId == reinterpret_cast<uintptr_t>(m_fontTexture.texture))
            {
                texture = (__bridge id<MTLTexture>)m_fontTexture.texture;
            }
            else if (texId > 0 && texId <= m_textures.size())
            {
                texture = (__bridge id<MTLTexture>)m_textures[texId - 1].texture;
            }
        }
        
        if (texture)
        {
            [encoder setFragmentTexture:texture atIndex:0];
        }

        // Draw indexed
        MTLIndexType indexType = sizeof(DrawIndex) == 2 ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32;
        NSUInteger indexBufferOffset = indexOffset * sizeof(DrawIndex);
        
        [encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                            indexCount:cmd.indexCount
                             indexType:indexType
                           indexBuffer:indexBuffer
                     indexBufferOffset:indexBufferOffset];

        indexOffset += cmd.indexCount;
    }
}

void MetalBackend::setupRenderState()
{
    id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)m_currentEncoder;

    // Set pipeline state
    id<MTLRenderPipelineState> pipelineState = (__bridge id<MTLRenderPipelineState>)m_pipelineState;
    [encoder setRenderPipelineState:pipelineState];

    // Set depth stencil state
    id<MTLDepthStencilState> depthState = (__bridge id<MTLDepthStencilState>)m_depthStencilState;
    [encoder setDepthStencilState:depthState];

    // Set sampler
    id<MTLSamplerState> samplerState = (__bridge id<MTLSamplerState>)m_samplerState;
    [encoder setFragmentSamplerState:samplerState atIndex:0];

    // Set viewport
    MTLViewport viewport;
    viewport.originX = 0;
    viewport.originY = 0;
    viewport.width = m_displayWidth;
    viewport.height = m_displayHeight;
    viewport.znear = 0.0;
    viewport.zfar = 1.0;
    [encoder setViewport:viewport];

    // Set cull mode
    [encoder setCullMode:MTLCullModeNone];

    // Set front facing
    [encoder setFrontFacingWinding:MTLWindingCounterClockwise];
}

void MetalBackend::ensureVertexBufferSize(u32 requiredSize)
{
    auto& frame = getCurrentFrameResources();
    
    if (frame.vertexBufferSize >= requiredSize)
        return;

    // Release old buffer
    if (frame.vertexBuffer)
    {
        CFRelease(frame.vertexBuffer);
        frame.vertexBuffer = nullptr;
    }

    // Create new buffer with extra capacity
    u32 newSize = requiredSize + 5000 * sizeof(DrawVertex);
    
    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device;
    id<MTLBuffer> buffer = [device newBufferWithLength:newSize
                                               options:MTLResourceStorageModeShared];
    
    frame.vertexBuffer = (__bridge_retained void*)buffer;
    frame.vertexBufferSize = newSize;
}

void MetalBackend::ensureIndexBufferSize(u32 requiredSize)
{
    auto& frame = getCurrentFrameResources();
    
    if (frame.indexBufferSize >= requiredSize)
        return;

    // Release old buffer
    if (frame.indexBuffer)
    {
        CFRelease(frame.indexBuffer);
        frame.indexBuffer = nullptr;
    }

    // Create new buffer with extra capacity
    u32 newSize = requiredSize + 10000 * sizeof(DrawIndex);
    
    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device;
    id<MTLBuffer> buffer = [device newBufferWithLength:newSize
                                               options:MTLResourceStorageModeShared];
    
    frame.indexBuffer = (__bridge_retained void*)buffer;
    frame.indexBufferSize = newSize;
}

MetalFrameResources& MetalBackend::getCurrentFrameResources()
{
    return m_frameResources[m_frameIndex];
}

TextureHandle MetalBackend::createTexture(const TextureCreateInfo& info)
{
    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device;

    // Determine pixel format
    MTLPixelFormat format = MTLPixelFormatRGBA8Unorm;
    switch (info.format)
    {
        case TextureFormat::RGBA8:
            format = MTLPixelFormatRGBA8Unorm;
            break;
        case TextureFormat::BGRA8:
            format = MTLPixelFormatBGRA8Unorm;
            break;
        case TextureFormat::R8:
            format = MTLPixelFormatR8Unorm;
            break;
        case TextureFormat::RG8:
            format = MTLPixelFormatRG8Unorm;
            break;
        case TextureFormat::RGBA16F:
            format = MTLPixelFormatRGBA16Float;
            break;
        case TextureFormat::RGBA32F:
            format = MTLPixelFormatRGBA32Float;
            break;
        default:
            break;
    }

    // Create texture descriptor
    MTLTextureDescriptor* descriptor = [[MTLTextureDescriptor alloc] init];
    descriptor.textureType = MTLTextureType2D;
    descriptor.pixelFormat = format;
    descriptor.width = info.width;
    descriptor.height = info.height;
    descriptor.mipmapLevelCount = info.generateMips ? 
        (NSUInteger)floor(log2(std::max(info.width, info.height))) + 1 : 1;
    descriptor.usage = MTLTextureUsageShaderRead;
    
    if (info.renderTarget)
        descriptor.usage |= MTLTextureUsageRenderTarget;

    descriptor.storageMode = MTLStorageModeShared;

    // Create texture
    id<MTLTexture> texture = [device newTextureWithDescriptor:descriptor];
    if (!texture)
        return {};

    // Upload initial data if provided
    if (info.initialData && info.initialDataSize > 0)
    {
        NSUInteger bytesPerRow = info.width;
        switch (info.format)
        {
            case TextureFormat::RGBA8:
            case TextureFormat::BGRA8:
                bytesPerRow = info.width * 4;
                break;
            case TextureFormat::R8:
                bytesPerRow = info.width;
                break;
            case TextureFormat::RG8:
                bytesPerRow = info.width * 2;
                break;
            case TextureFormat::RGBA16F:
                bytesPerRow = info.width * 8;
                break;
            case TextureFormat::RGBA32F:
                bytesPerRow = info.width * 16;
                break;
            default:
                break;
        }

        MTLRegion region = MTLRegionMake2D(0, 0, info.width, info.height);
        [texture replaceRegion:region 
                   mipmapLevel:0
                     withBytes:info.initialData
                   bytesPerRow:bytesPerRow];
    }

    // Track texture
    MetalTextureData texData;
    texData.texture = (__bridge_retained void*)texture;
    texData.width = info.width;
    texData.height = info.height;
    texData.format = static_cast<u32>(format);

    uintptr_t id = m_nextTextureId++;
    m_textures.push_back(texData);

    return TextureHandle{id, info.width, info.height, static_cast<u32>(info.format)};
}

void MetalBackend::updateTexture(TextureHandle texture, const void* data, usize dataSize,
                                  u32 x, u32 y, u32 width, u32 height)
{
    if (!texture.isValid() || texture.id == 0 || texture.id > m_textures.size())
        return;

    auto& texData = m_textures[texture.id - 1];
    id<MTLTexture> mtlTexture = (__bridge id<MTLTexture>)texData.texture;

    if (width == 0) width = texture.width;
    if (height == 0) height = texture.height;

    NSUInteger bytesPerRow = width;
    switch (static_cast<TextureFormat>(texture.format))
    {
        case TextureFormat::RGBA8:
        case TextureFormat::BGRA8:
            bytesPerRow = width * 4;
            break;
        case TextureFormat::R8:
            bytesPerRow = width;
            break;
        case TextureFormat::RG8:
            bytesPerRow = width * 2;
            break;
        default:
            break;
    }

    MTLRegion region = MTLRegionMake2D(x, y, width, height);
    [mtlTexture replaceRegion:region
                  mipmapLevel:0
                    withBytes:data
                  bytesPerRow:bytesPerRow];
}

void MetalBackend::destroyTexture(TextureHandle texture)
{
    if (!texture.isValid() || texture.id == 0 || texture.id > m_textures.size())
        return;

    auto& texData = m_textures[texture.id - 1];
    if (texData.texture)
    {
        CFRelease(texData.texture);
        texData.texture = nullptr;
    }
}

TextureHandle MetalBackend::createFontTexture(u32 width, u32 height, const u8* pixels)
{
    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device;

    MTLTextureDescriptor* descriptor = [[MTLTextureDescriptor alloc] init];
    descriptor.textureType = MTLTextureType2D;
    descriptor.pixelFormat = MTLPixelFormatR8Unorm;
    descriptor.width = width;
    descriptor.height = height;
    descriptor.usage = MTLTextureUsageShaderRead;
    descriptor.storageMode = MTLStorageModeShared;

    id<MTLTexture> texture = [device newTextureWithDescriptor:descriptor];
    if (!texture)
        return {};

    MTLRegion region = MTLRegionMake2D(0, 0, width, height);
    [texture replaceRegion:region
               mipmapLevel:0
                 withBytes:pixels
               bytesPerRow:width];

    m_fontTexture.texture = (__bridge_retained void*)texture;
    m_fontTexture.width = width;
    m_fontTexture.height = height;
    m_fontTexture.format = MTLPixelFormatR8Unorm;

    return TextureHandle{reinterpret_cast<uintptr_t>(m_fontTexture.texture), width, height,
                         static_cast<u32>(TextureFormat::R8)};
}

void MetalBackend::setViewport(i32 x, i32 y, i32 width, i32 height)
{
    if (!m_currentEncoder)
        return;

    id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)m_currentEncoder;
    
    MTLViewport viewport;
    viewport.originX = x;
    viewport.originY = y;
    viewport.width = width;
    viewport.height = height;
    viewport.znear = 0.0;
    viewport.zfar = 1.0;
    
    [encoder setViewport:viewport];

    m_viewportX = static_cast<f32>(x);
    m_viewportY = static_cast<f32>(y);
    m_viewportWidth = static_cast<f32>(width);
    m_viewportHeight = static_cast<f32>(height);
}

void MetalBackend::setScissorRect(i32 x, i32 y, i32 width, i32 height)
{
    if (!m_currentEncoder)
        return;

    id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)m_currentEncoder;
    
    MTLScissorRect scissor;
    scissor.x = static_cast<NSUInteger>(std::max(0, x));
    scissor.y = static_cast<NSUInteger>(std::max(0, y));
    scissor.width = static_cast<NSUInteger>(std::max(0, width));
    scissor.height = static_cast<NSUInteger>(std::max(0, height));
    
    [encoder setScissorRect:scissor];
}

void MetalBackend::invalidateDeviceState()
{
    // Metal doesn't need state invalidation
}

}  // namespace dakt::gui

#else  // !__APPLE__

// Stub implementation for non-Apple platforms
namespace dakt::gui
{

MetalBackend::MetalBackend() = default;
MetalBackend::~MetalBackend() = default;

void MetalBackend::configure(const MetalBackendConfig&) {}
bool MetalBackend::initialize() { return false; }
void MetalBackend::shutdown() {}
void MetalBackend::beginFrame(i32, i32) {}
void MetalBackend::endFrame() {}
void MetalBackend::renderDrawList(const DrawList&) {}
void MetalBackend::renderDrawLists(std::span<const DrawList*>) {}
TextureHandle MetalBackend::createTexture(const TextureCreateInfo&) { return {}; }
void MetalBackend::updateTexture(TextureHandle, const void*, usize, u32, u32, u32, u32) {}
void MetalBackend::destroyTexture(TextureHandle) {}
TextureHandle MetalBackend::createFontTexture(u32, u32, const u8*) { return {}; }
void MetalBackend::setViewport(i32, i32, i32, i32) {}
void MetalBackend::setScissorRect(i32, i32, i32, i32) {}
void MetalBackend::setRenderPassDescriptor(void*) {}
void MetalBackend::setDrawable(void*) {}
void MetalBackend::invalidateDeviceState() {}

}  // namespace dakt::gui

#endif  // __APPLE__