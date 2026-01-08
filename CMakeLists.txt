# =============================================================================
# DaktLib-GUI - Zero-dependency, cross-platform GUI library
# =============================================================================
# C++23 | CMake 4.2.1+ | Single-file build configuration
# Platforms: WIndows (x86/x64), Linux (x64), macOS (x64/ARM64)
# Backends: DX11, DX12, Vulkan, OpenGL 4.5+, Metal
# =============================================================================

cmake_minimum_required(VERSION 4.2.1)

# -----------------------------------------------------------------------------
# Project Definition
# -----------------------------------------------------------------------------
project(DaktLib-GUI
    VERSION 0.1.0
    DESCRIPTION "Zero-dependency, cross-platform GUI library with multi-backend rendering"
    HOMEPAGE_URL "https://github.com/Schuldakt/DaktLib-GUI"
    LANGUAGES CXX
)

# -----------------------------------------------------------------------------
# Build Options
# -----------------------------------------------------------------------------
option(DAKTLIB_GUI_BUILD_SHARED     "Build shared library"              ON)
option(DAKTLIB_GUI_BUILD_STATIC     "Build static library"              ON)
option(DAKTLIB_GUI_BUILD_TESTS      "Build test suite"                  ON)
option(DAKTLIB_GUI_BUILD_EXAMPLES   "Build example application"         ON)
option(DAKTLIB_GUI_ENABLE_VULKAN    "Enable Vulkan backend"             ON)
option(DAKTLIB_GUI_ENABLE_OPENGL    "Enable OpenGL backend"             ON)
option(DAKTLIB_GUI_ENABLE_DX11      "Enable DirectX 11 backend (Win)"   ON)
option(DAKTLIB_GUI_ENABLE_DX12      "Enable DirectX 12 backend (Win)"   ON)
option(DAKTLIB_GUI_ENABLE_METAL     "Enable Metal backend (macOS)"      ON)
option(DAKTLIB_GUI_STRICT_WARNINGS  "Enable strict compiler warnings"   ON)

# -----------------------------------------------------------------------------
# C++ Standard & Compiler Settings
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# -----------------------------------------------------------------------------
# Platform & Architecture Detection
# -----------------------------------------------------------------------------
set(DAKTLIB_PLATFORM_WINDOWS OFF)
set(DAKTLIB_PLATFORM_LINUX OFF)
set(DAKTLIB_PLATFORM_MACOS OFF)

if (WIN32)
    set(DAKTLIB_PLATFORM_WINDOWS ON)
    set(DAKTLIB_PLATFORM_NAME "Windows")
elseif(APPLE)
    set(DAKTLIB_PLATFORM_MACOS ON)
    set(DAKTLIB_PLATFORM_NAME "macOS")
elseif(UNIX)
    set(DAKTLIB_PLATFORM_LINUX ON)
    set(DAKTLIB_PLATFORM_NAME "Linux")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Architecture detection
set(DAKTLIB_ARCH_X86 OFF)
set(DAKTLIB_ARCH_X64 OFF)
set(DAKTLIB_ARCH_ARM64 OFF)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64|ARM64")
        set(DAKTLIB_ARCH_ARM64 ON)
        set(DAKTLIB_ARCH_NAME "ARM64")
    else()
        set(DAKTLIB_ARCH_X64 ON)
        set(DAKTLIB_ARCH_NAME "x64")
    endif()
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(DAKTLIB_ARCH_X86 ON)
    set(DAKTLIB_ARCH_NAME "x86")
else()
    message(FATAL_ERROR "Unsupported architecture: point size ${CMAKE_SIZE_OF_VOID_P}")
endif()

message(STATUS "DaktLib-GUI v${PROJECT_VERSION}")
message(STATUS "    Platform: ${DAKTLIB_PLATFORM_NAME}")
message(STATUS "    Architecture: ${DAKTLIB_ARCH_NAME}")
message(STATUS "    Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# -----------------------------------------------------------------------------
# Version Configuration
# -----------------------------------------------------------------------------
math(EXPR DAKTLIB_GUI_VERSION_NUMBER
    "${PROJECT_VERSION_MAJOR} * 10000 + ${PROJECT_VERSION_MINOR} * 100 + ${PROJECT_VERSION_PATCH}")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/dakt/gui/Version.hpp"
    @ONLY
)

# -----------------------------------------------------------------------------
# Compile Definitions & Options
# -----------------------------------------------------------------------------
set(DAKTLIB_COMPILE_DEFINITIONS "")

# Platform definitions
if(DAKTLIB_PLATFORM_WINDOWS)
    list(APPEND DAKTLIB_COMPILE_DEFINITIONS
        DAKTLIB_PLATFORM_WINDOWS=1
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
    if(DAKTLIB_ARCH_x64)
        list(APPEND DAKTLIB_COMPILE_DEFINITIONS _AMD64_ WIN64 DAKTLIB_ARCH_X64=1)
    else()
        list(APPEND DAKTLIB_COMPILE_DEFINITIONS _X86_ WIN32 DAKTLIB_ARCH_X86=1)
    endif()
elseif(DAKTLIB_PLATFORM_MACOS)
    list(APPEND DAKTLIB_COMPILE_DEFINITIONS DAKTLIB_PLATFORM_MACOS=1)
    if(DAKTLIB_ARCH_ARM64)
        list(APPEND DAKTLIB_COMPILE_DEFINITIONS DAKTLIB_ARCH_ARM64=1)
    else()
        list(APPEND DAKTLIB_COMPILE_DEFINITIONS DAKTLIB_ARCH_X64=1)
    endif()
elseif(DAKTLIB_PLATFORM_LINUX)
    list(APPEND DAKTLIB_COMPILE_DEFINITIONS DAKTLIB_PLATFORM_LINUX=1 DAKTLIB_ARCH_X64=1)
endif()

# Compiler-specific options
set(DAKTLIB_COMPILE_OPTIONS "")
if(DAKTLIB_GUI_STRICT_WARNINGS)
    if(MSVC)
        list(APPEND DAKTLIB_COMPILE_OPTIONS
            /W4 /permissive- /Zc:__cplusplus /Zc:preprocessor /EHsc /utf-8
        )
        if(DAKTLIB_ARCH_X64)
            list(APPEND DAKTLIB_COMPILE_OPTIONS /arch:AVX2)
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        list(APPEND DAKTLIB_COMPILE_OPTIONS
            -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wconversion -Wshadow
        )
        if(DAKTLIB_ARCH_X64)
            list(APPEND DAKTLIB_COMPILE_OPTIONS -m64)
        elseif(DAKTLIB_ARCH_X86)
            list(APPEND DAKTLIB_COMPILE_OPTIONS -m32)
        endif()
    endif()
endif()

# -----------------------------------------------------------------------------
# Source Files
# -----------------------------------------------------------------------------
set(DAKTLIB_GUI_SOURCES
    # Core
    src/core/Types.cpp
    src/core/Context.cpp
    src/core/Frame.cpp

    # Layout
    src/subsystems/layout/Layout.cpp
    src/subsystems/layout/FlexLayout.cpp

    # Style
    src/subsystems/style/Style.cpp
    src/subsystems/style/Theme.cpp

    # Input
    src/subsystems/input/Input.cpp
    src/subsystems/input/HitTest.cpp

    # Animation
    src/animation/Animation.cpp
    src/animation/Timeline.cpp

    # Text
    src/subsystems/text/Text.cpp
    src/subsystems/text/Font.cpp
    src/subsystems/text/GlyphAtlas.cpp
    src/subsystems/text/GlyphCache.cpp
    src/subsystems/text/TextShaper.cpp
    src/subsystems/text/TextCursor.cpp
    src/subsystems/text/TTFParser.cpp
    src/subsystems/text/SDFGenerator.cpp
    src/subsystems/text/VariableFont.cpp
    src/subsystems/text/OTFParser.cpp

    # Draw
    src/subsystems/draw/DrawList.cpp
    src/subsystems/draw/DrawBatcher.cpp

    # Immediate Mode
    src/immediate/Immediate.cpp

    # Immediate Mode Containers
    src/immediate/Containers/Child.cpp
    src/immediate/Containers/Group.cpp
    src/immediate/Containers/Layout.cpp
    src/immediate/Containers/Window.cpp

    # Immediate Mode Internal
    src/immediate/internal/ImmediateState.cpp
    src/immediate/internal/ImmediateStateAccess.cpp

    # Immediate Mode Widgets
    src/immediate/Widgets/Button.cpp
    src/immediate/Widgets/Checkbox.cpp
    src/immediate/Widgets/Color.cpp
    src/immediate/Widgets/InputText.cpp
    src/immediate/Widgets/ListBox.cpp
    src/immediate/Widgets/Menu.cpp
    src/immediate/Widgets/Popup.cpp
    src/immediate/Widgets/ProgressBar.cpp
    src/immediate/Widgets/Selectable.cpp
    src/immediate/Widgets/Slider.cpp
    src/immediate/Widgets/Table.cpp
    src/immediate/Widgets/Text.cpp
    src/immediate/Widgets/Tooltip.cpp
    src/immediate/Widgets/Tree.cpp

    # Retained Mode - Core
    src/retained/UIContainer.cpp
    src/retained/Widget.cpp

    # Retained Mode - Widgets
    src/retained/widgets/Shape.cpp
    src/retained/widgets/ProgressBar.cpp
    src/retained/widgets/RadioButton.cpp
    src/retained/widgets/Dropdown.cpp
    src/retained/widgets/ColorPicker.cpp
    src/retained/widgets/Menu.cpp
    src/retained/widgets/Table.cpp
    src/retained/widgets/TextInputEnhanced.cpp

    # Retained Mode - Containers
    src/retained/containers/Grid.cpp
    src/retained/containers/Stack.cpp
    src/retained/containers/Splitter.cpp
    src/retained/containers/Wrap.cpp

    # Backend Abstraction
    src/backend/Backend.cpp

    # C API
    src/c_api/dakt_gui.cpp
)

# -----------------------------------------------------------------------------
# Object Library
# -----------------------------------------------------------------------------
add_library(DaktLib-GUI_obj OBJECT ${DAKTLIB_GUI_SOURCES})

target_include_directories(DaktLib-GUI_obj
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(DaktLib-GUI_obj PRIVATE ${DAKTLIB_COMPILE_DEFINITIONS} DAKTLIB_GUI_EXPORTS)
target_compile_options(DaktLib-GUI_obj PRIVATE ${DAKTLIB_COMPILE_OPTIONS})
target_compile_features(DaktLib-GUI_obj PUBLIC cxx_std_23)

# -----------------------------------------------------------------------------
# Backend Configuration
# -----------------------------------------------------------------------------
set(DAKTLIB_BACKEND_LIBRARIES "")

# DirectX 11 (Windows)
if(DAKTLIB_PLATFORM_WINDOWS AND DAKTLIB_GUI_ENABLE_DX11)
    target_sources(DaktLib-GUI_obj PRIVATE
        src/backend/dx11/DX11Backend.cpp
        src/backend/dx11/Resources.cpp
        src/backend/dx11/Rendering.cpp
    )
    list(APPEND DAKTLIB_BACKEND_LIBRARIES d3d11 dxgi d3dcompiler)
    target_compile_definitions(DaktLib-GUI_obj PRIVATE DAKTLIB_ENABLE_DX11=1)
    message(STATUS "    Backend: DirectX 11 ✓")
endif()

if(DAKTLIB_PLATFORM_WINDOWS)
    list(APPEND DAKTLIB_BACKEND_LIBRARIES ws2_32)
endif()

# DirectX 12 (Windows)
if(DAKTLIB_PLATFORM_WINDOWS AND DAKTLIB_GUI_ENABLE_DX12)
    target_sources(DaktLib-GUI_obj PRIVATE
        src/backend/dx12/DX11Backend.cpp
        src/backend/dx12/Resources.cpp
        src/backend/dx12/Rendering.cpp
    )
    list(APPEND DAKTLIB_BACKEND_LIBRARIES d3d12)
    target_compile_definitions(DaktLib-GUI_obj PRIVATE DAKTLIB_ENABLE_DX12=1)
    message(STATUS "    Backend: DirectX 12 ✓")
endif()

if(DAKTLIB_PLATFORM_WINDOWS)
    list(APPEND DAKTLIB_BACKEND_LIBRARIES ws2_32)
endif()

# Vulkan (cross-platform)
if(DAKTLIB_GUI_ENABLE_VULKAN)
    find_package(Vulkan QUIET)
    if(Vulkan_FOUND)
        target_sources(DaktLib-GUI_obj PRIVATE
            src/backend/vulkan/VulkanBackend.cpp
            src/backend/vulkan/Resources.cpp
            src/backend/vulkan/Rendering.cpp
        )
        target_include_directories(DaktLib-GUI_obj PRIVATE ${Vulkan_INCLUDE_DIRS})
        list(APPEND DAKTLIB_BACKEND_LIBRARIES Vulkan::Vulkan)
        target_compile_definitions(DaktLib-GUI_obj PRIVATE DAKTLIB_ENABLE_VULKAN=1)
        message(STATUS "    Backend: Vulkan ✓")
    else()
        message(STATUS "    Backend: Vulkan ✗ (not found)")
    endif()
endif()

# OpenGL (cross-platform)
if(DAKTLIB_GUI_ENABLE_OPENGL)
    target_sources(DaktLib-GUI_obj PRIVATE
        src/backend/opengl/OpenGLBackend.cpp
        src/backend/opengl/Resources.cpp
        src/backend/opengl/Rendering.cpp
    )
    target_compile_definitions(DaktLib-GUI_obj PRIVATE DAKTLIB_ENABLE_OPENGL=1)
    message(STATUS "    Backend: OpenGL ✓")
endif()

# Metal (macOS)
if(DAKTLIB_PLATFORM_MACOS AND DAKTLIB_GUI_ENABLE_METAL)
    enable_language(OBJCXX)
    target_sources(DaktLib-GUI_obj PRIVATE
        src/backend/metal/MetalBackend.mm
        src/backend/metal/Resources.mm
        src/backend/metal/Rendering.mm
    )
    list(APPEND DAKTLIB_BACKEND_LIBRARIES
        "-framework Metal" "-framework MetalKit" "-framework Cocoa" "-framework QuartzCore"
    )
    target_compile_definitions(DaktLib-GUI_obj PRIVATE DAKTLIB_ENABLE_METAL=1)
    message(STATUS "    Backend: Metal ✓")
endif()

target_link_libraries(DaktLib-GUI_obj PRIVATE ${DAKTLIB_BACKEND_LIBRARIES})

# -----------------------------------------------------------------------------
# Library Targets
# -----------------------------------------------------------------------------
function(daktlib_create_library TARGET_NAME LIB_TYPE OUTPUT_NAME)
    add_library(${TARGET_NAME} ${LIB_TYPE} $<TARGET_OBJECTS:DaktLib-GUI_obj>)
    target_include_directories(${TARGET_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    target_compile_definitions(${TARGET_NAME} PUBLIC ${DAKTLIB_COMPILE_DEFINITIONS})
    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_23)
    target_link_libraries(${TARGET_NAME} PUBLIC ${DAKTLIB_BACKEND_LIBRARIES})
    set_target_properties(${TARGET_NAME} PROPERTIES
        OUTPUT_NAME "${OUTPUT_NAME}"
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endfunction()

if(DAKTLIB_GUI_BUILD_SHARED)
    daktlib_create_library(DaktLib-GUI_shared SHARED DaktLib-GUI)
    target_compile_definitions(DaktLib-GUI_shared PRIVATE DAKTLIB_GUI_EXPORTS)
    add_library(dakt::gui ALIAS DaktLib-GUI_shared)
endif()

if(DAKTLIB_GUI_BUILD_STATIC)
    daktlib_create_library(DaktLib-GUI_static STATIC DaktLib-GUI_static)
    target_compile_definitions(DaktLib-GUI_static PUBLIC DAKTLIB_GUI_STATIC)
    if(NOT TARGET dakt::gui)
        add_library(dakt::gui ALIAS DaktLib-GUI_static)
    endif()
endif()

# -----------------------------------------------------------------------------
# Installation
# -----------------------------------------------------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(DAKTLIB_INSTALL_TARGETS "")
if(TARGET DaktLib-GUI_shared)
    list(APPEND DAKTLIB_INSTALL_TARGETS DaktLib-GUI_shared)
endif()
if(TARGET DaktLib-GUI_static)
    list(APPEND DAKTLIB_INSTALL_TARGETS DaktLib-GUI_static)
endif()

install(TARGETS ${DAKTLIB_INSTALL_TARGETS}
    EXPORT DaktLib-GUITargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

# Install the C API header for consumers (ensures c_api.h is always present)
install(FILES include/dakt/gui/c_api/c_api.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dakt/gui/c_api)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/dakt/gui/Version.hpp"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dakt/gui
)

install(EXPORT DaktLib-GUITargets
    FILE DaktLib-GUITargets.cmake
    NAMESPACE dakt::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/daktgui
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DaktLib-GUIConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/DaktLib-GUIConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DaktLib-GUI
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/DaktLib-GUIConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/DaktLib-GUIConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/DaktLib-GUIConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DaktLib-GUI
)

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
if(DAKTLIB_GUI_BUILD_TESTS)
    enable_testing()
    set(DAKTLIB_TESTING_LIBRARY $<IF:$<TARGET_EXISTS:DaktLib-GUI_static>,DaktLib-GUI_static,DaktLib-GUI_shared>)

    foreach(PHASE IN ITEMS phase1 phase2 phase3)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/${PHASE}_tests.cpp")
            add_executable(DaktLib-GUI_${PHASE}_tests tests/${PHASE}_tests.cpp)
            target_link_libraries(DaktLib-GUI_${PHASE}_tests PRIVATE ${DAKTLIB_TESTING_LIBRARY})
            add_test(NAME DaktLib-GUI_${PHASE}_tests COMMAND DaktLib-GUI_${PHASE}_tests)
        endif()
    endforeach()
endif()

# -----------------------------------------------------------------------------
# Examples
# -----------------------------------------------------------------------------
if(DAKTLIB_GUI_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
endif()


