cmake_minimum_required(VERSION 4.2.1)

project(DaktLibGUI VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_SHARED_LIBS "Build shared library" ON)
option(DAKT_GUI_BUILD_TESTS "Build tests" ON)
option(DAKT_GUI_BUILD_SHOWCASE "Build showcase example" ON)

set(DAKT_GUI_SOURCES
    src/core/Context.cpp
    src/core/Frame.cpp
    src/core/Types.cpp
    src/immediate/Immediate.cpp
    src/retained/UIContainer.cpp
    src/retained/Widget.cpp
    src/layout/Layout.cpp
    src/style/Style.cpp
    src/input/Input.cpp
    src/animation/Animation.cpp
    src/draw/DrawList.cpp
    src/text/Text.cpp
    src/backend/Vulkan.cpp
    src/backend/OpenGL.cpp
    src/c_api/c_api.cpp
)

if(WIN32)
    list(APPEND DAKT_GUI_SOURCES
        src/backend/DX11.cpp
        src/backend/DX12.cpp
    )
endif()

if(APPLE)
    list(APPEND DAKT_GUI_SOURCES src/backend/Metal.mm)
    set_source_files_properties(src/backend/Metal.mm PROPERTIES LANGUAGE OBJCXX)
endif()

add_library(daktgui ${DAKT_GUI_SOURCES})

target_include_directories(daktgui
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(daktgui PUBLIC cxx_std_23)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(daktgui PUBLIC DAKT_GUI_USE_SHARED)
    target_compile_definitions(daktgui PRIVATE DAKT_GUI_BUILD_SHARED)
endif()

if(MSVC)
    target_compile_options(daktgui PRIVATE /W4 /permissive-)
else()
    target_compile_options(daktgui PRIVATE -Wall -Wextra -Wpedantic)
endif()

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${DAKT_GUI_SOURCES})

if(DAKT_GUI_BUILD_TESTS)
    include(CTest)
    add_executable(daktgui_tests tests/smoke.cpp)
    target_link_libraries(daktgui_tests PRIVATE daktgui)
    target_compile_features(daktgui_tests PRIVATE cxx_std_23)
    add_test(NAME daktgui.smoke COMMAND daktgui_tests)
endif()

if(DAKT_GUI_BUILD_SHOWCASE)
    add_executable(daktgui_showcase examples/showcase.cpp)
    target_link_libraries(daktgui_showcase PRIVATE daktgui)
    target_compile_features(daktgui_showcase PRIVATE cxx_std_23)
endif()
