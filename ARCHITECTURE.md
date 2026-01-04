# DaktLib-GUI Architecture

> Custom immediate-first GUI with optional retained containers. Five render backends: DirectX 11, DirectX 12, Vulkan, OpenGL 4.5+, Metal. No external deps.

## Overview

```
┌─────────────────────────────────────────────────────────────┐
│                      Application Layer                      │
├─────────────────────────────────────────────────────────────┤
│           Immediate API          │     Retained Tree        │
│  (BeginWindow, Button, Slider)   │  (UIContainer, Widget)   │
├─────────────────────────────────────────────────────────────┤
│  Input System  │  Layout (Flex)  │  Animation  │  Styling   │
├─────────────────────────────────────────────────────────────┤
│      DrawList / Command Buffer / RenderState Management     │
├─────────────────────────────────────────────────────────────┤
│         Backend Abstraction (IRenderBackend Interface)      │
├───────────────┬───────────────────┬───────────────┬─────────┤
│    DX11       │      DX12         │    Vulkan     │ OpenGL  │
│   (Win10+)    │     (Win10+)      │  (Win/Linux)  │ 4.5+    │
├───────────────┴───────────────────┴───────────────┴─────────┤
│                        Metal (macOS)                        │
└─────────────────────────────────────────────────────────────┘
```

## Namespace Structure

```cpp
namespace dakt::gui {
    // Frontend APIs
    class Context;
    class Frame;  // per-frame state
    class Immediate; // BeginWindow/EndWindow, widgets
    class UIContainer; // retained root
    class Widget; // retained widgets

    // Layout
    class FlexLayout;
    class LayoutNode;

    // Styling & Theme
    struct ColorScheme;
    struct Theme;
    class Style;

    // Input
    struct InputState;
    class InputSystem;

    // Animation
    class Animator;
    struct Easing;

    // Drawing
    class DrawList;
    struct DrawCommand;
    struct Vertex;
    struct RenderState;

    // Text & Fonts
    class FontAtlas;
    class Font;
    class TextShaper; // SDF + shaping

    // Backend abstraction
    class IRenderBackend;

    namespace backend {
        class DX11;
        class DX12;
        class Vulkan;
        class OpenGL;
        class Metal;
    }
}
```

## Directory Structure

```
DaktLib-GUI/
├── include/dakt/gui/
│   ├── GUI.hpp                 # Main include
│   ├── Context.hpp
│   ├── Immediate.hpp
│   ├── UIContainer.hpp
│   ├── Widget.hpp
│   ├── Layout.hpp
│   ├── Style.hpp
│   ├── Input.hpp
│   ├── Animation.hpp
│   ├── DrawList.hpp
│   ├── Text.hpp
│   ├── backend/
│   │   ├── IRenderBackend.hpp
│   │   ├── DX11.hpp
│   │   ├── DX12.hpp
│   │   ├── Vulkan.hpp
│   │   ├── OpenGL.hpp
│   │   └── Metal.hpp
│   └── c_api.h
├── src/
│   ├── Context.cpp
│   ├── Immediate.cpp
│   ├── UIContainer.cpp
│   ├── Layout.cpp
│   ├── Style.cpp
│   ├── Input.cpp
│   ├── Animation.cpp
│   ├── DrawList.cpp
│   ├── Text.cpp
│   ├── backend/
│   │   ├── DX11.cpp
│   │   ├── DX12.cpp
│   │   ├── Vulkan.cpp
│   │   ├── OpenGL.cpp
│   │   └── Metal.mm
│   └── c_api.cpp
├── shaders/            # HLSL/GLSL/MSL sources
├── tests/
├── CMakeLists.txt
├── ARCHITECTURE.md
└── TODO.md
```

## Core Concepts

### Immediate Mode API
- Stateless call sequence per frame: `BeginFrame`, `BeginWindow`, widgets, `EndWindow`, `EndFrame`.
- Produces `DrawList` command stream consumed by the backend.
- IDs derived from labels or explicit `PushID/PopID`.

### Retained Mode (Optional)
- `UIContainer` holds a tree of `Widget` objects with persistent state.
- Bridges to immediate API via a virtual `build(Context&)` override.
- Useful for complex views (navigation, data grids) while keeping immediate ergonomics for simple overlays.

### Layout System (Flex-inspired)
- `LayoutNode` with flex properties: `flex-direction`, `justify-content`, `align-items`, `flex-grow`, `flex-shrink`, `flex-basis`.
- Supports wrapping, min/max sizes, absolute positioning for overlays.
- Measurements cached per frame; dirty-prop tags for minimal recompute.

### Styling & Theme
- `Theme` holds colors, spacing, radii, shadows, typography scale.
- State-driven variants (hover, active, disabled, focus) resolved before rendering.
- Global and per-widget style overrides; supports dark/light themes.

### Input System
- Aggregates mouse, keyboard, gamepad; supports IME text input.
- Hit-testing resolved against layout tree; focus management, tab traversal.
- Per-frame `InputState` fed into immediate API; handles clicks, drags, double-clicks, key repeats.

### Animation
- `Animator` ticks easing curves (`linear`, `quad`, `cubic`, `elastic`, `bounce`).
- Property channels (position, size, color, opacity, rotation) driven by time.
- Timeline support for sequences and staggering.

### Draw Pipeline
- `DrawList` records triangles, lines, convex/concave polygons, rounded rects, circles, ellipses, hexagons.
- Batching by texture + render state; minimizes state changes.
- Clip rect stack for nested windows and scissor regions.
- Optional command merging for identical states.

### Text Rendering
- Signed Distance Field (SDF) atlas with MSDF option for sharper corners.
- Text shaping pipeline (bidi, kerning, ligatures) with fallback to bitmap if shaping is unavailable (configurable).
- Cursor/selection rendering for text inputs; caret blinking managed by animation system.

### Backend Abstraction (`IRenderBackend`)
- Interface methods: `createResources`, `beginFrame`, `submit(const DrawList&)`, `present`, `resize`, `shutdown`.
- Separate shader variants per backend; shader compilation offline to bytecode (DXIL, SPIR-V, MSL, GLSL).
- Resource handles (buffers, textures, samplers) opaque to core.

### Windowing & Surfaces
- Uses host window/swapchain provided by application; also supports offscreen render-to-texture.
- DPI awareness; device-independent pixels in layout, scaled per monitor.

### Accessibility (A11y) Hooks
- Node metadata (role, name, state) exportable to platform a11y bridges (future phase).

## C API Sketch

```c
// dakt_gui.h

typedef struct DaktGuiContext* DaktGuiContextHandle;
typedef struct DaktGuiBackend* DaktGuiBackendHandle;

typedef struct {
    float x, y;
    float width, height;
} DaktGuiRect;

typedef struct {
    int key;
    int down;
} DaktGuiKeyEvent;

typedef struct {
    int button;
    int down;
    float x, y;
} DaktGuiMouseEvent;

typedef struct {
    const char* text;
} DaktGuiTextEvent;

DAKT_API DaktGuiContextHandle dakt_gui_create(DaktGuiBackendHandle backend);
DAKT_API void dakt_gui_destroy(DaktGuiContextHandle ctx);

DAKT_API void dakt_gui_new_frame(DaktGuiContextHandle ctx, float dt);
DAKT_API void dakt_gui_process_mouse(DaktGuiContextHandle ctx, const DaktGuiMouseEvent* ev);
DAKT_API void dakt_gui_process_key(DaktGuiContextHandle ctx, const DaktGuiKeyEvent* ev);
DAKT_API void dakt_gui_process_text(DaktGuiContextHandle ctx, const DaktGuiTextEvent* ev);

DAKT_API int dakt_gui_begin_window(DaktGuiContextHandle ctx, const char* title, DaktGuiRect rect, int flags);
DAKT_API void dakt_gui_end_window(DaktGuiContextHandle ctx);
DAKT_API int dakt_gui_button(DaktGuiContextHandle ctx, const char* label);
DAKT_API int dakt_gui_checkbox(DaktGuiContextHandle ctx, const char* label, int* value);
DAKT_API int dakt_gui_slider_float(DaktGuiContextHandle ctx, const char* label, float* v, float min, float max);

DAKT_API void dakt_gui_render(DaktGuiContextHandle ctx);
```

## Thread Safety

| Component | Safety |
|-----------|--------|
| Context | Not thread-safe (single main thread) |
| DrawList | Frame-local; single producer |
| FontAtlas build | Single-threaded; can be prebuilt offline |
| Backend | One thread per context; no cross-thread device calls |

## Performance Considerations

1. **Batching**: Merge draw calls by texture/state; atlas UI textures.
2. **Clip Hierarchy**: Minimal scissor changes using stack-based clip regions.
3. **GPU Resources**: Persistent vertex/index buffers with ring allocator.
4. **Text**: SDF/MSDF reduce atlas size; reuse glyph layouts per font size.
5. **Animations**: CPU-side; avoid per-vertex morphing.
6. **DPI**: Layout in DIPs, snap to pixel grid to avoid blurriness.

## Platform Backends

| Backend | API | Notes |
|---------|-----|-------|
| DX11 | IDXGISwapChain | Windows 10+, simplest path |
| DX12 | IDXGISwapChain3 | Descriptor heaps, fences |
| Vulkan | VK_KHR_swapchain | Shared shader set via SPIR-V |
| OpenGL | 4.5 core | FBO-based; VAOs/VBOs |
| Metal | CAMetalLayer | Separate MSL shaders |

## Compliance

- No hooks or process injection; renders to app-provided surfaces.
- All shaders and assets are owned by the library; no external dependencies.
