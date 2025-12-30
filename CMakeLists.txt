# GUI Module - Custom immediate-mode GUI system
# Dependencies: Core, Logger, Events

add_library(DaktGUI STATIC
    # Core types and primitives
    src/Types.cpp
    src/DrawList.cpp
    src/Font.cpp
    
    # Widget framework
    src/Context.cpp
    src/Layout.cpp
    src/Input.cpp
    
    # Basic widgets
    src/Widgets.cpp
    
    # Container widgets
    src/Containers.cpp
    
    # Data widgets
    src/DataWidgets.cpp
    
    # Theming
    src/Theme.cpp
    
    # Render backends
    src/D3D11Backend.cpp
)

target_include_directories(DaktGUI
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(DaktGUI
    PUBLIC
        DaktCore
        DaktLogger
        DaktEvents
    PRIVATE
        d3d11
        dxgi
        d3dcompiler
)

target_compile_features(DaktGUI PUBLIC cxx_std_23)

add_library(Dakt::GUI ALIAS DaktGUI)
