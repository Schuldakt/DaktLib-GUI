# DaktLib-GUI TODO

## Legend
- **Priority**: P0 (Critical) → P3 (Nice-to-have)
- **Complexity**: S (Small) | M (Medium) | L (Large) | XL (Extra Large)

---

## Phase 1: Foundation
- [ ] **[S]** Modular tree scaffolding (core/immediate/retained/layout/style/input/animation/draw/text/backend/c_api)
- [ ] **[S]** CMake scaffolding (C++23, presets, `CMAKE_EXPORT_COMPILE_COMMANDS`)
- [ ] **[S]** Core types (`Context`, `Frame`, `DrawList`, `RenderState`, export macros)
- [ ] **[M]** Vertex/index buffer formats (pos2, uv, color; optional SDF data)
- [ ] **[S]** Color utilities, rect math, DPI helpers

## Phase 2: Input System
- [ ] **[M]** `InputState` (mouse, keyboard, gamepad)
- [ ] **[S]** Key repeat, double-click, drag detection
- [ ] **[S]** IME text input plumbing

## Phase 3: Layout
- [ ] **[M]** Flex layout engine (direction, wrap, justify, align)
- [ ] **[M]** Size constraints (min/max/basis), grow/shrink factors
- [ ] **[S]** Absolute positioning for overlays
- [ ] **[S]** Layout caching & dirty propagation

## Phase 4: Styling & Theme
- [ ] **[S]** `Theme` structure (colors, spacing, radii, shadows)
- [ ] **[M]** State variants (hover/active/disabled/focus)
- [ ] **[S]** Global + per-widget overrides

## Phase 5: Text & Fonts
- [ ] **[L]** FontAtlas builder (SDF/MSDF), offline bake path
- [ ] **[M]** Text shaping (kerning, bidi) fallback toggle
- [ ] **[M]** Text rendering primitive (cursor, selection)
- [ ] **[S]** Font fallback chain

## Phase 6: Immediate API
- [ ] **[M]** `BeginFrame/EndFrame`
- [ ] **[M]** Windows (move/resize/scroll/clip)
- [ ] **[M]** Basic widgets: label, button, checkbox, slider, combo, input text, progress bar
- [ ] **[S]** ID stack helpers

## Phase 7: Retained API (Optional)
- [ ] **[M]** `UIContainer` + `Widget` base
- [ ] **[M]** Virtual `build()` bridge to immediate API
- [ ] **[S]** State storage per widget

## Phase 8: Animation System
- [ ] **[M]** Easing functions and channels
- [ ] **[S]** Timeline/sequencing helper
- [ ] **[S]** Hover/press animations on core widgets

## Phase 9: DrawList & Primitives
- [ ] **[M]** Command buffer with batching
- [ ] **[M]** Shapes: rect, rounded rect, circle, ellipse, hexagon, polygon, line, polyline
- [ ] **[S]** Clip stack
- [ ] **[S]** Path stroking and fills

## Phase 10: Backends
- [ ] **[L]** DX11 backend (swapchain, shaders, buffers)
- [ ] **[L]** DX12 backend (descriptor heaps, fences)
- [ ] **[L]** Vulkan backend (pipelines, descriptor sets)
- [ ] **[M]** OpenGL backend (VAO/VBO/FBO)
- [ ] **[M]** Metal backend (CAMetalLayer, MTLRenderPipeline)
- [ ] **[S]** Shader compilation pipeline (DXIL/SPIR-V/MSL/GLSL)

## Phase 11: Text Inputs & A11y Hooks
- [ ] **[M]** Text input widget with selection, clipboard
- [ ] **[S]** Caret blinking driven by animation system
- [ ] **[S]** Basic accessibility metadata (role, name, state) export hook

## Phase 12: Theming & Assets
- [ ] **[S]** Default dark/light themes
- [ ] **[S]** Icon atlas support (packed textures)
- [ ] **[S]** User theme override API

## Phase 13: C API
- [ ] **[S]** Handle-based context creation/destruction
- [ ] **[S]** Input event forwarding
- [ ] **[M]** Core widgets exposed (button, checkbox, slider, window)
- [ ] **[S]** Render call bridging to backend

## Phase 14: Testing & Samples
- [ ] **[M]** Unit tests: layout, color, math, input edge cases
- [ ] **[M]** Golden-image tests for widgets
- [ ] **[S]** Sample app per backend
- [ ] **[S]** Benchmark: widget stress, text throughput

## Milestones

| Milestone | Target | Phases |
|-----------|--------|--------|
| **v0.1.0** | Week 4 | 1-4 (foundation, input, layout, styling) |
| **v0.2.0** | Week 8 | 5-6 (text, immediate widgets) |
| **v0.3.0** | Week 12 | 7-9 (retained, animation, primitives) |
| **v0.4.0** | Week 18 | 10 (DX11/DX12/Vulkan/GL) |
| **v0.5.0** | Week 24 | 10 (Metal) + 11 (text input) |
| **v0.6.0** | Week 28 | 12-13 (theming, C API) |
| **v1.0.0** | Week 32-36 | 14 (tests/samples) |

## Estimated Effort
- Core + widgets + 3 backends: ~24-28 weeks
- Metal + polish + tests: +6-8 weeks
- Total: ~30-36 weeks

## Acceptance Criteria
- [ ] Renders identical UI across all backends (visual parity)
- [ ] Widgets respond to input at 60+ FPS
- [ ] DPI scaling correct on multi-monitor setups
- [ ] Text shaping works for Latin; fallback path togglable
- [ ] No external dependencies; static/dynamic builds succeed on Win/Linux/macOS
- [ ] C API usable from C# via ClangSharp
