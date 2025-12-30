# GUI Module Architecture

## Overview

Immediate-mode GUI system built from scratch.

**Dependencies:** Core, Logger, Events

## Components

```
┌─────────────────────────────────────────────────────────────────────┐
│                           GUI Module                                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                      DaktGui Integration                    │    │
│  │  - Context management                                       │    │
│  │  - Render backend abstraction                               │    │
│  │  - Input handling                                           │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                              │                                      │
│         ┌────────────────────┼────────────────────┐                 │
│         ▼                    ▼                    ▼                 │
│  ┌─────────────┐     ┌─────────────┐     ┌─────────────┐            │
│  │   D3D11     │     │   D3D12     │     │   OpenGL    │            │
│  │  Backend    │     │  Backend    │     │  Backend    │            │
│  └─────────────┘     └─────────────┘     └─────────────┘            │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                    Custom Widgets                           │    │
│  │  PropertyGrid  │  TreeList   │  DataTable  │  HexView       │    │
│  │  ImageView     │  Timeline   │  NodeEditor │  CodeEditor    │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                      Theming                                │    │
│  │  - Color schemes                                            │    │
│  │  - Font management                                          │    │
│  │  - Style presets                                            │    │
│  └─────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────┘
```

## Custom Widgets

| Widget | Description |
|--------|-------------|
| PropertyGrid | Key-value editor with types |
| TreeList | Tree + list hybrid view |
| DataTable | Sortable/filterable table |
| HexView | Binary hex editor |
| ImageView | Texture viewer with zoom |
| Timeline | Animation timeline |
| NodeEditor | Node-based editor |
| CodeEditor | Syntax highlighting editor |

## Render Backends

- **D3D11** - DirectX 11 (primary for Windows)
- **D3D12** - DirectX 12 (optional)
- **OpenGL** - OpenGL 3.3+ (cross-platform)
- **Vulkan** - Optional for advanced use

## Key Features

- DaktGui integration
- Multiple render backends
- Custom high-productivity widgets
- Theming and styling
- Font management (TTF/OTF)
- DPI-aware scaling
- Docking and multi-viewport

## API Preview

```cpp
GuiContext gui;
gui.initialize(window, GuiBackend::D3D11);

// Main loop
while (running) {
    gui.beginFrame();
    
    if (DaktGui::Begin("Properties")) {
        PropertyGrid::show(selectedObject);
    }
    DaktGui::End();
    
    if (DaktGui::Begin("Hex View")) {
        HexView::show(buffer.span());
    }
    DaktGui::End();
    
    gui.endFrame();
    gui.render();
}
```

## External Dependencies

- **None**