cmake_minimum_required(VERSION 3.23)

project(DaktLibGUI VERSION 0.1.0 LANGUAGES CXX)

option(DAKTGUI_BUILD_SHARED "Build shared library" ON)
option(DAKTGUI_BUILD_STATIC "Build static library" ON)
option(DAKTGUI_BUILD_TESTS "Build tests" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DAKTGUI_SOURCES
    # Core
    src/core/Types.cpp
    src/core/Context.cpp
    src/core/Frame.cpp
    
    # Layout
    src/layout/Layout.cpp
    src/layout/FlexLayout.cpp
    
    # Style
    src/style/Style.cpp
    src/style/Theme.cpp
    
    # Input
    src/input/Input.cpp
    src/input/HitTest.cpp
    
    # Animation
    src/animation/Animation.cpp
    src/animation/Timeline.cpp
    
    # Text
    src/text/Text.cpp
    src/text/Font.cpp
    src/text/GlyphAtlas.cpp
    src/text/GlyphCache.cpp
    src/text/TextShaper.cpp
    src/text/TextCursor.cpp
    src/text/TTFParser.cpp
    src/text/SDFGenerator.cpp
    src/text/VariableFont.cpp
    src/text/OTFParser.cpp
    
    # Draw
    src/draw/DrawList.cpp
    src/draw/DrawBatcher.cpp
    
    # Immediate
    src/immediate/Immediate.cpp
    src/immediate/Widgets.cpp
    
    # Retained - Core
    src/retained/UIContainer.cpp
    src/retained/Widget.cpp
    
    # Retained - Widgets
    src/retained/widgets/Shape.cpp
    src/retained/widgets/ProgressBar.cpp
    src/retained/widgets/RadioButton.cpp
    src/retained/widgets/Dropdown.cpp
    src/retained/widgets/ColorPicker.cpp
    src/retained/widgets/Menu.cpp
    src/retained/widgets/Table.cpp
    src/retained/widgets/TextInputEnhanced.cpp
    
    # Retained - Containers
    src/retained/containers/Grid.cpp
    src/retained/containers/Stack.cpp
    src/retained/containers/Splitter.cpp
    src/retained/containers/Wrap.cpp
    
    # Backend abstraction
    src/backend/Backend.cpp
    
    # C API
    src/c_api/Context.cpp
    src/c_api/Input.cpp
    src/c_api/Widgets.cpp
)

add_library(daktgui_obj OBJECT ${DAKTGUI_SOURCES})

target_include_directories(daktgui_obj
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(daktgui_obj PRIVATE DAKT_GUI_EXPORTS)

target_compile_features(daktgui_obj PUBLIC cxx_std_23)

# Enable position-independent code for shared libraries
set_target_properties(daktgui_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Platform-specific handling via compile definitions (handled in code)
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_compile_definitions(daktgui_obj PRIVATE _AMD64_ _WIN64 WIN64 DAKT_WINDOWS=1 DAKT_ARCH_X64=1)
    else()
        target_compile_definitions(daktgui_obj PRIVATE _X86_ WIN32 DAKT_WINDOWS=1 DAKT_ARCH_X86=1)
    endif()
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            target_compile_options(daktgui_obj PRIVATE -m64)
        else()
            target_compile_options(daktgui_obj PRIVATE -m32)
        endif()
    endif()
elseif(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        target_compile_definitions(daktgui_obj PRIVATE DAKT_MACOS=1 DAKT_ARCH_ARM64=1)
    else()
        target_compile_definitions(daktgui_obj PRIVATE DAKT_MACOS=1 DAKT_ARCH_X64=1)
    endif()
elseif(UNIX)
    target_compile_definitions(daktgui_obj PRIVATE DAKT_LINUX=1 DAKT_ARCH_X64=1)
endif()

# Conditionally add backend implementations
if(WIN32)
    # DX11 backend
    target_sources(daktgui_obj PRIVATE
        src/backend/dx11/DX11Backend.cpp
        src/backend/dx11/Resources.cpp
        src/backend/dx11/Rendering.cpp
    )
    target_link_libraries(daktgui_obj PRIVATE d3d11 dxgi d3dcompiler)
    target_compile_definitions(daktgui_obj PRIVATE DAKT_ENABLE_DX11=1)
    
    # DX12 backend
    target_sources(daktgui_obj PRIVATE
        src/backend/dx12/DX12Backend.cpp
        src/backend/dx12/Resources.cpp
        src/backend/dx12/Rendering.cpp
    )
    target_link_libraries(daktgui_obj PRIVATE d3d12)
    target_compile_definitions(daktgui_obj PRIVATE DAKT_ENABLE_DX12=1)
    
    target_link_libraries(daktgui_obj PRIVATE ws2_32)
endif()

# Vulkan backend (cross-platform)
find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    message(STATUS "Vulkan found: ${Vulkan_LIBRARY}")
    target_sources(daktgui_obj PRIVATE
        src/backend/vulkan/VulkanBackend.cpp
        src/backend/vulkan/Resources.cpp
        src/backend/vulkan/Rendering.cpp
    )
    target_include_directories(daktgui_obj PRIVATE ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(daktgui_obj PRIVATE Vulkan::Vulkan)
    target_compile_definitions(daktgui_obj PRIVATE DAKT_ENABLE_VULKAN=1)
else()
    message(STATUS "Vulkan not found - Vulkan backend disabled")
endif()

# OpenGL backend (cross-platform)
target_sources(daktgui_obj PRIVATE
    src/backend/opengl/OpenGLBackend.cpp
    src/backend/opengl/Resources.cpp
    src/backend/opengl/Rendering.cpp
)
target_compile_definitions(daktgui_obj PRIVATE DAKT_ENABLE_OPENGL=1)

# Metal backend (macOS only)
if(APPLE)
    enable_language(OBJCXX)
    target_sources(daktgui_obj PRIVATE
        src/backend/metal/MetalBackend.mm
        src/backend/metal/Resources.mm
        src/backend/metal/Rendering.mm
    )
    target_link_libraries(daktgui_obj PRIVATE
        "-framework Metal"
        "-framework MetalKit"
        "-framework Cocoa"
        "-framework QuartzCore"
    )
    target_compile_definitions(daktgui_obj PRIVATE DAKT_ENABLE_METAL=1)
endif()

# Collect all link libraries that need to be propagated
set(DAKTGUI_LINK_LIBRARIES "")

if(Vulkan_FOUND)
    list(APPEND DAKTGUI_LINK_LIBRARIES Vulkan::Vulkan)
endif()

if(APPLE)
    list(APPEND DAKTGUI_LINK_LIBRARIES
        "-framework Metal"
        "-framework MetalKit"
        "-framework Cocoa"
        "-framework QuartzCore"
    )
endif()

if(WIN32)
    list(APPEND DAKTGUI_LINK_LIBRARIES d3d11 dxgi d3dcompiler d3d12 ws2_32)
endif()

function(daktgui_promote_library target_name lib_type output_name)
    add_library(${target_name} ${lib_type} $<TARGET_OBJECTS:daktgui_obj>)
    target_include_directories(${target_name}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    # Link libraries that were used by the object library
    target_link_libraries(${target_name} PRIVATE ${DAKTGUI_LINK_LIBRARIES})
    set_target_properties(${target_name} PROPERTIES OUTPUT_NAME "${output_name}")
endfunction()

if(DAKTGUI_BUILD_SHARED)
    daktgui_promote_library(daktgui_shared SHARED daktgui)
    target_compile_definitions(daktgui_shared PRIVATE DAKT_GUI_EXPORTS)
endif()

if(DAKTGUI_BUILD_STATIC)
    daktgui_promote_library(daktgui_static STATIC daktgui_static)
    target_compile_definitions(daktgui_static PUBLIC DAKT_GUI_STATIC)
endif()

install(TARGETS daktgui_obj
        EXPORT daktguiTargets
        INCLUDES DESTINATION include)

if(TARGET daktgui_shared)
    install(TARGETS daktgui_shared
            EXPORT daktguiTargets
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            INCLUDES DESTINATION include)
endif()

if(TARGET daktgui_static)
    install(TARGETS daktgui_static
            EXPORT daktguiTargets
            ARCHIVE DESTINATION lib
            INCLUDES DESTINATION include)
endif()

install(DIRECTORY include/ DESTINATION include)

install(EXPORT daktguiTargets
        FILE daktguiTargets.cmake
        NAMESPACE dakt::
        DESTINATION lib/cmake/daktgui)

# Generate and install CMake config files
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/daktguiConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/daktguiConfig.cmake"
    INSTALL_DESTINATION lib/cmake/daktgui
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/daktguiConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/daktguiConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/daktguiConfigVersion.cmake"
    DESTINATION lib/cmake/daktgui
)

# Version file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/dakt/gui/Version.hpp"
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/include/dakt/gui/Version.hpp"
    DESTINATION include/dakt/gui
)

if(DAKTGUI_BUILD_TESTS)
    enable_testing()
    
    # Phase 1 tests
    add_executable(daktgui_tests
        tests/phase1_tests.cpp
    )
    if(TARGET daktgui_static)
        target_link_libraries(daktgui_tests PRIVATE daktgui_static)
    elseif(TARGET daktgui_shared)
        target_link_libraries(daktgui_tests PRIVATE daktgui_shared)
    endif()
    add_test(NAME daktgui_tests COMMAND daktgui_tests)
    
    # Phase 2 tests
    add_executable(daktgui_phase2_tests
        tests/phase2_tests.cpp
    )
    if(TARGET daktgui_static)
        target_link_libraries(daktgui_phase2_tests PRIVATE daktgui_static)
    elseif(TARGET daktgui_shared)
        target_link_libraries(daktgui_phase2_tests PRIVATE daktgui_shared)
    endif()
    add_test(NAME daktgui_phase2_tests COMMAND daktgui_phase2_tests)
    
    # Phase 3 tests
    add_executable(daktgui_phase3_tests
        tests/phase3_tests.cpp
    )
    if(TARGET daktgui_static)
        target_link_libraries(daktgui_phase3_tests PRIVATE daktgui_static)
    elseif(TARGET daktgui_shared)
        target_link_libraries(daktgui_phase3_tests PRIVATE daktgui_shared)
    endif()
    add_test(NAME daktgui_phase3_tests COMMAND daktgui_phase3_tests)
    
    # Phase 4 tests
    add_executable(daktgui_phase4_tests
        tests/phase4_tests.cpp
    )
    if(TARGET daktgui_static)
        target_link_libraries(daktgui_phase4_tests PRIVATE daktgui_static)
    elseif(TARGET daktgui_shared)
        target_link_libraries(daktgui_phase4_tests PRIVATE daktgui_shared)
    endif()
    add_test(NAME daktgui_phase4_tests COMMAND daktgui_phase4_tests)
endif()

# Examples (optional, requires GLFW)
add_subdirectory(examples)
