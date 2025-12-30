# ============================================================================
# DaktLib GUI Module - Custom immediate-mode GUI system
# ============================================================================
# Dependencies: Core, Logger, Events
# 
# Rendering Backends:
#   - D3D11, D3D12 (Windows)
#   - Metal (macOS, iOS)
#   - OpenGL (Cross-platform)
#   - Vulkan (Cross-platform, except Apple without MoltenVK)
# ============================================================================

cmake_minimum_required(VERSION 3.20)

# ============================================================================
# Source Files
# ============================================================================

set(DAKT_GUI_CORE_SOURCES
    src/core/Types.cpp
    src/core/DrawList.cpp
    src/core/Font.cpp
)

set(DAKT_GUI_FRAMEWORK_SOURCES
    src/framework/Context.cpp
    src/framework/Layout.cpp
    src/framework/Input.cpp
    src/framework/Theme.cpp
)

set(DAKT_GUI_WIDGET_SOURCES
    src/widgets/Widgets.cpp
    src/widgets/Containers.cpp
    src/widgets/DataWidgets.cpp
)

set(DAKT_GUI_BACKEND_SOURCES
    src/backends/RenderBackend.cpp
    src/backends/OpenGLBackend.cpp
    src/backends/VulkanBackend.cpp
)

# Platform-specific backend sources
if(WIN32)
    list(APPEND DAKT_GUI_BACKEND_SOURCES
        src/backends/D3D11Backend.cpp
        src/backends/D3D12Backend.cpp
    )
elseif(APPLE)
    list(APPEND DAKT_GUI_BACKEND_SOURCES
        src/backends/MetalBackend.mm
    )
endif()

# Combine all sources
set(DAKT_GUI_SOURCES
    ${DAKT_GUI_CORE_SOURCES}
    ${DAKT_GUI_FRAMEWORK_SOURCES}
    ${DAKT_GUI_WIDGET_SOURCES}
    ${DAKT_GUI_BACKEND_SOURCES}
)

# ============================================================================
# Library Target
# ============================================================================

add_library(DaktGUI STATIC ${DAKT_GUI_SOURCES})

target_include_directories(DaktGUI
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(DaktGUI
    PUBLIC
        DaktCore
)

target_compile_features(DaktGUI PUBLIC cxx_std_23)

# ============================================================================
# Platform-Specific Configuration
# ============================================================================

if(WIN32)
    # DirectX dependencies
    target_link_libraries(DaktGUI
        PRIVATE
            d3d11
            d3d12
            dxgi
            d3dcompiler
    )
    target_compile_definitions(DaktGUI PRIVATE DAKT_HAS_D3D11=1 DAKT_HAS_D3D12=1)
endif()

if(APPLE)
    # Metal framework
    find_library(METAL_FRAMEWORK Metal REQUIRED)
    find_library(METALKIT_FRAMEWORK MetalKit REQUIRED)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
    
    target_link_libraries(DaktGUI
        PRIVATE
            ${METAL_FRAMEWORK}
            ${METALKIT_FRAMEWORK}
            ${QUARTZCORE_FRAMEWORK}
    )
    target_compile_definitions(DaktGUI PRIVATE DAKT_HAS_METAL=1)
    
    # Enable Objective-C++ for .mm files
    set_source_files_properties(
        src/backends/MetalBackend.mm
        PROPERTIES
            COMPILE_FLAGS "-x objective-c++"
    )
endif()

# ============================================================================
# Optional Dependencies
# ============================================================================

# Vulkan (optional, cross-platform)
find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    target_link_libraries(DaktGUI PRIVATE Vulkan::Vulkan)
    target_compile_definitions(DaktGUI PRIVATE DAKT_HAS_VULKAN=1)
    message(STATUS "DaktGUI: Vulkan support enabled")
else()
    message(STATUS "DaktGUI: Vulkan not found, Vulkan backend disabled")
endif()

# OpenGL (optional, cross-platform)
find_package(OpenGL QUIET)
if(OpenGL_FOUND)
    target_link_libraries(DaktGUI PRIVATE OpenGL::GL)
    target_compile_definitions(DaktGUI PRIVATE DAKT_HAS_OPENGL=1)
    message(STATUS "DaktGUI: OpenGL support enabled")
else()
    message(STATUS "DaktGUI: OpenGL not found, OpenGL backend disabled")
endif()

# ============================================================================
# Alias Target
# ============================================================================

add_library(Dakt::GUI ALIAS DaktGUI)

# ============================================================================
# Installation
# ============================================================================

install(TARGETS DaktGUI
    EXPORT DaktLibTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# ============================================================================
# Summary
# ============================================================================

message(STATUS "")
message(STATUS "DaktGUI Configuration:")
message(STATUS "  Core sources:      ${DAKT_GUI_CORE_SOURCES}")
message(STATUS "  Framework sources: ${DAKT_GUI_FRAMEWORK_SOURCES}")
message(STATUS "  Widget sources:    ${DAKT_GUI_WIDGET_SOURCES}")
message(STATUS "  Backend sources:   ${DAKT_GUI_BACKEND_SOURCES}")
message(STATUS "")