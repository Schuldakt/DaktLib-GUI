cmake_minimum_required(VERSION 4.2)

# Core library
add_library(DaktLib-GUI)

# Headers
target_include_directories(DaktLib-GUI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_BINARY_DIR}/../include
)

# Source files - organized by subsystem
target_sources(DaktLib-GUI PRIVATE
    # Core
    core/Types.cpp
    core/Context.cpp
    core/Frame.cpp
    
    # Layout
    layout/Layout.cpp
    layout/FlexLayout.cpp
    
    # Style
    style/Style.cpp
    style/Theme.cpp
    
    # Input
    input/Input.cpp
    input/HitTest.cpp
    
    # Animation
    animation/Animation.cpp
    animation/Timeline.cpp
    
    # Text
    text/Text.cpp
    text/Font.cpp
    text/GlyphAtlas.cpp
    text/TextShaper.cpp
    text/TTFParser.cpp
    # text/OTFParser.cpp
    # text/VariableFont.cpp
    # text/SDFGenerator.cpp
    # text/GlyphCache.cpp
    # text/TextCursor.cpp
    
    # Draw
    draw/DrawList.cpp
    draw/DrawBatcher.cpp
    
    # Immediate
    immediate/Immediate.cpp
    immediate/Widgets.cpp
    
    # Retained
    retained/UIContainer.cpp
    retained/Widget.cpp
    
    # Backend abstraction
    backend/Backend.cpp
)

# Conditionally add backend implementations
if(DAKT_GUI_ENABLE_VULKAN)
    target_sources(DaktLib-GUI PRIVATE
        backend/vulkan/VulkanBackend.cpp
        backend/vulkan/Resources.cpp
        backend/vulkan/Rendering.cpp
    )
    target_compile_definitions(DaktLib-GUI PRIVATE DAKT_ENABLE_VULKAN=1)
endif()

if(DAKT_GUI_ENABLE_DX11 AND WIN32)
    target_sources(DaktLib-GUI PRIVATE
        backend/dx11/DX11Backend.cpp
        backend/dx11/Resources.cpp
        backend/dx11/Rendering.cpp
    )
    target_link_libraries(DaktLib-GUI PRIVATE d3d11 dxgi d3dcompiler)
    target_compile_definitions(DaktLib-GUI PRIVATE DAKT_ENABLE_DX11=1)
endif()

if(DAKT_GUI_ENABLE_DX12 AND WIN32)
    target_sources(DaktLib-GUI PRIVATE
        backend/dx12/DX12Backend.cpp
        backend/dx12/Resources.cpp
        backend/dx12/Rendering.cpp
    )
    target_link_libraries(DaktLib-GUI PRIVATE d3d12 dxgi d3dcompiler)
    target_compile_definitions(DaktLib-GUI PRIVATE DAKT_ENABLE_DX12=1)
endif()

if(DAKT_GUI_ENABLE_OPENGL)
    target_sources(DaktLib-GUI PRIVATE
        backend/opengl/OpenGLBackend.cpp
        backend/opengl/Resources.cpp
        backend/opengl/Rendering.cpp
    )
    target_compile_definitions(DaktLib-GUI PRIVATE DAKT_ENABLE_OPENGL=1)
endif()

if(DAKT_GUI_ENABLE_METAL AND APPLE)
    enable_language(OBJCXX)
    target_sources(DaktLib-GUI PRIVATE
        backend/metal/MetalBackend.mm
        backend/metal/Resources.mm
        backend/metal/Rendering.mm
    )
    target_link_libraries(DaktLib-GUI PRIVATE
        "-framework Metal"
        "-framework MetalKit"
        "-framework Cocoa"
        "-framework QuartzCore"
    )
    target_compile_definitions(DaktLib-GUI PRIVATE DAKT_ENABLE_METAL=1)
endif()

# C API
target_sources(DaktLib-GUI PRIVATE
    c_api/Context.cpp
    c_api/Input.cpp
    c_api/Widgets.cpp
)

# Platform-specific definitions
if(DAKT_PLATFORM_WINDOWS)
    target_compile_definitions(DaktLib-GUI PRIVATE DAKT_WINDOWS=1)
    target_link_libraries(DaktLib-GUI PRIVATE ws2_32)
elseif(DAKT_PLATFORM_MACOS)
    target_compile_definitions(DaktLib-GUI PRIVATE DAKT_MACOS=1)
elseif(DAKT_PLATFORM_LINUX)
    target_compile_definitions(DaktLib-GUI PRIVATE DAKT_LINUX=1)
endif()

# Architecture definitions
if(DAKT_ARCH_X86)
    target_compile_definitions(DaktLib-GUI PRIVATE DAKT_ARCH_X86=1)
elseif(DAKT_ARCH_X64)
    target_compile_definitions(DaktLib-GUI PRIVATE DAKT_ARCH_X64=1)
elseif(DAKT_ARCH_ARM64)
    target_compile_definitions(DaktLib-GUI PRIVATE DAKT_ARCH_ARM64=1)
endif()

# Library properties
set_target_properties(DaktLib-GUI PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
)

if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(DaktLib-GUI PROPERTIES
        PREFIX ""
    )
endif()
# Testing
enable_testing()
add_executable(phase1_tests_exe ../tests/phase1_tests.cpp)

target_include_directories(phase1_tests_exe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_BINARY_DIR}/../include
)

target_link_libraries(phase1_tests_exe PRIVATE
    DaktLib-GUI
)

add_test(NAME Phase1Tests COMMAND phase1_tests_exe)