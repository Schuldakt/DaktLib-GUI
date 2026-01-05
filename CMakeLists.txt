cmake_minimum_required(VERSION 4.2)

project(DaktLib-GUI 
    VERSION 0.1.0 
    LANGUAGES CXX
    DESCRIPTION "Zero-dependency immediate-first GUI library with Vulkan rendering"
)

# C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} /permissive-")

# Disable C++20 module scanning for now (clang issues)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Export compile commands for clangd/IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries")
option(DAKT_GUI_BUILD_TESTS "Build test suite" ON)
option(DAKT_GUI_BUILD_SHOWCASE "Build showcase example" ON)
option(DAKT_GUI_ENABLE_VULKAN "Enable Vulkan backend" ON)
option(DAKT_GUI_ENABLE_DX11 "Enable Direct3D 11 backend" ON)
option(DAKT_GUI_ENABLE_DX12 "Enable Direct3D 12 backend" ON)
option(DAKT_GUI_ENABLE_OPENGL "Enable OpenGL backend" ON)
option(DAKT_GUI_ENABLE_METAL "Enable Metal backend" OFF)

# Platform detection
if(WIN32)
    set(DAKT_PLATFORM_WINDOWS ON)
    if(CMAKE_GENERATOR_PLATFORM MATCHES "ARM64")
        set(DAKT_ARCH_ARM64 ON)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(DAKT_ARCH_X86 ON)
    else()
        set(DAKT_ARCH_X64 ON)
    endif()
elseif(APPLE)
    set(DAKT_PLATFORM_MACOS ON)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        set(DAKT_ARCH_ARM64 ON)
    else()
        set(DAKT_ARCH_X64 ON)
    endif()
elseif(UNIX AND NOT APPLE)
    set(DAKT_PLATFORM_LINUX ON)
    set(DAKT_ARCH_X64 ON)
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/Wall /permissive- /std:c++latest)
    add_compile_options(/utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(NOT WIN32)
        add_compile_options(-fPIC)
    endif()
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Add source subdirectories
add_subdirectory(src)

# Examples
if(DAKT_GUI_BUILD_SHOWCASE)
    add_subdirectory(examples)
endif()

# Installation targets
install(
    DIRECTORY include/dakt
    DESTINATION include
)

install(
    TARGETS DaktLib-GUI
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
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

message(STATUS "DaktLib-GUI Configuration:")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared Libraries: ${BUILD_SHARED_LIBS}")
message(STATUS "  Vulkan Backend: ${DAKT_GUI_ENABLE_VULKAN}")
message(STATUS "  DX11 Backend: ${DAKT_GUI_ENABLE_DX11}")
message(STATUS "  DX12 Backend: ${DAKT_GUI_ENABLE_DX12}")
message(STATUS "  OpenGL Backend: ${DAKT_GUI_ENABLE_OPENGL}")
message(STATUS "  Metal Backend: ${DAKT_GUI_ENABLE_METAL}")
