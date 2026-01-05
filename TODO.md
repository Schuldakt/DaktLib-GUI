# DaktLib-GUI Implementation Plan

Zero-dependency, cross-platform GUI library with native font generation, Vulkan-first rendering, and C# bindings.
Built with C++23, CMake 4.2.1, releases for Windows (x86/x64), Linux, and macOS.

## Progress Summary

| Phase | Description | Status |
|-------|-------------|--------|
| 0 | Project Scaffold & Build System | In Progress |
| 1 | Core Types, Layout, Input & DrawList | ✅ Complete |
| 2 | Text Rendering & Font System | ✅ Complete |
| 3 | Vulkan Backend & Shaders | ✅ Complete |
| 4 | Retained Mode & Advanced Widgets | ✅ Complete |
| 5 | C# Bindings & Interop | Not Started |
| 6 | Documentation & Demos | Not Started |

---

## Phase 0: Project Scaffold & Build System

### Infrastructure Setup

- [x] **Folder structure created** — All directories scaffold (include/, src/, shaders/, tests/, examples/, bindings/, etc.)
- [x] **Root CMakeLists.txt configured** — C++23, Ninja generator, platform detection, backend selection
- [x] **src/CMakeLists.txt created** — Target definitions, conditional backend compilation
- [ ] **Version management** — Version.hpp.in template for versioning

### Toolchain & Cross-Compilation

- [ ] **Windows x86 toolchain** — /arch:IA32 or -m32 flags
- [ ] **Windows x64 toolchain** — /arch:AVX2 or -m64 flags
- [ ] **Linux x64 toolchain** — GCC/Clang x86_64 targeting
- [ ] **macOS x64 toolchain** — Clang ARM64/x86_64 universal builds
- [ ] **CMake architecture detection** — DAKT_ARCH_* defines for runtime checks

### CI/CD Workflows

- [ ] **GitHub Actions self-hosted runner setup** — Document Windows x86/x64, Linux, macOS provisioning
- [ ] **Build workflow (build.yml)** — Matrix: [Windows x86/x64, Linux x64, macOS x64/arm64], Ninja builds, artifact upload
- [ ] **Test workflow (test.yml)** — Run test suites on each platform matrix
- [ ] **Release workflow (release.yml)** — Trigger on version tags, package static libs + headers + C# bindings
- [ ] **NuGet publish workflow** — Publish DaktLib.GUI NuGet package on release

### Packaging & Distribution

- [ ] **DaktLib.GUI.nuspec created** — Single NuGet package with runtime-specific natives (win-x86, win-x64, linux-x64, osx-x64, osx-arm64)
- [ ] **CMake install targets** — Install headers, static libs, C# bindings to standard locations
- [ ] **Release artifact generation** — Automated packaging: static libs per platform, include/, C# bindings

### Self-Hosted Runner Provisioning

- [ ] **RUNNER_SETUP.md** — Step-by-step guides:
  - [ ] Windows x86 agent setup (Actions self-hosted, register, environment)
  - [ ] Windows x64 agent setup
  - [ ] Linux agent setup (docker/native)
  - [ ] macOS x64 agent setup
  - [ ] macOS ARM64 agent setup (Apple Silicon)
  - [ ] GitHub token/secret management for NuGet API
- [ ] **Runner environment validation** — CMake, Ninja, compiler versions documented

---

## Phase 1: Core Foundation ✅

### Core Types & Structures

- [x] **Types.hpp created** — Primitive types, ID generation (UUID v4), 2D vectors (float, int), colors (RGBA), rectangles, alignment enums
- [x] **Types.cpp stub** — ID generation implementation, color utilities

### Context & Lifecycle

- [x] **Context.hpp designed** — Context struct: frame state, input buffer, draw list, backend reference, theme, animator pool
- [x] **Context.cpp implemented** — Constructor, destructor, frame lifecycle
- [x] **Frame.hpp designed** — Per-frame input, layout tree cache, draw batches, dirty flags
- [x] **Frame.cpp implemented** — Reset between frames, accumulate input, propagate dirty flags

### Layout System

- [x] **Layout.hpp designed** — LayoutNode: size, position, flex properties (direction, justify, align, grow, shrink, basis), min/max, absolute position, dirty flags
- [x] **FlexLayout.cpp algorithm** — Flex box implementation: measure pass, layout pass, wrapping, min/max constraints
- [x] **Dirty-prop caching** — Mark nodes as dirty only when properties change, batch relayout

### Styling & Theming

- [x] **Style.hpp designed** — State enum (normal, hover, active, disabled, focus), style struct (padding, margin, border, radius)
- [x] **Theme.hpp created** — Colors (primary, secondary, bg, text), spacing scales, typography (font family, sizes, weights), shadows
- [x] **Theme.cpp state variants** — Apply state-driven color/shadow overrides

### Input Aggregation

- [x] **Input.hpp designed** — Mouse (pos, buttons, wheel), keyboard (keys, text input), gamepad, IME structures
- [x] **Input.cpp aggregator** — Collect per-frame input, manage focus/hover/active states
- [x] **HitTest.cpp** — Recursive tree traversal, Z-order/priority, click/drag/double-click routing

### Animation System (moved from Phase 5)

- [x] **Animation.hpp** — Animator: property channels (position, size, color, opacity), duration, easing function
- [x] **Animation.cpp** — Tween template, Spring physics, easing curves (Linear, Quad, Cubic, Elastic, Bounce, Back, Expo)
- [x] **Timeline.cpp** — Animation timeline with keyframes

### Immediate Mode API (moved from Phase 4)

- [x] **Immediate.hpp** — BeginFrame(deltaTime), EndFrame(), window/panel scope functions, ID stack
- [x] **Immediate.cpp** — Frame state machine, ID stack (PushID/PopID), label-based ID hashing, item state tracking
- [x] **Widgets.cpp** — Button, Checkbox, RadioButton, SliderFloat/Int, ProgressBar, Text, ColorButton, Selectable, ListBox
- [x] **Item state functions** — isItemHovered, isItemActive, isItemEdited, isItemActivated, isItemDeactivated
- [x] **Focus management** — setItemDefaultFocus, setKeyboardFocusHere
- [x] **Double-click detection** — isMouseDoubleClicked with time/distance thresholds

### Draw Commands (moved from Phase 3)

- [x] **DrawList.hpp designed** — Command types: Triangle, Line, Rect, RoundedRect, Circle
- [x] **DrawList.cpp implementation** — Record commands, vertex/index buffers, outline rounded rect
- [x] **DrawBatcher.cpp** — Basic batching structure

### Tests (Phase 1)

- [x] **Layout tests** — Flex algorithm correctness (horizontal, vertical, wrapping)
- [x] **Input tests** — Focus management, hit-testing, state transitions
- [x] **Type tests** — ID generation uniqueness, color conversions

---

## Phase 2: Native Font Generation & Text Rendering ✅

### TrueType/OpenType Parsing

- [x] **TTFParser.cpp** — Parse HEAD, HHEA, HMTX, GLYF, CMAP tables; extract glyph outlines (composite, simple)
- [x] **OTFParser.cpp** — Parse CFF table, support PostScript outlines alongside TrueType
- [x] **Binary format readers** — Big-endian integer/fixed parsing, safe bounds checking

### Variable Fonts

- [x] **FVAR table parsing** — Read axis definitions (weight 100–900, width 50–200, slant, custom)
- [x] **GVAR table parsing** — Glyph deltas for each axis
- [x] **AVAR table parsing** — Axis value normalizations
- [x] **Interpolation** — Blend glyphs across multi-dimensional axis space

### SDF/MSDF Atlas Generation

- [x] **SDFGenerator.cpp** — Rasterize glyphs to signed distance field, edge distance computation
- [x] **MSDF support** — Multi-channel SDF with color per edge, sharper rendering at any size
- [x] **Atlas packing** — Efficient 2D bin packing (skyline algorithm), minimize wastage
- [x] **Prebuilt atlas format** — Save/load .daktfont files (glyph metrics, atlas texture, axis mappings)

### Text Shaping

- [x] **TextShaper.cpp** — Apply GSUB/GPOS tables (ligatures, kerning, contextual substitution)
- [x] **Bidi support** — Bidirectional text reordering for RTL languages (simplified UAX #9)
- [x] **Ligature substitution** — f+i → fi, f+l → fl, etc.
- [x] **Kerning** — Apply kern pair adjustments from font

### Glyph Caching

- [x] **GlyphCache.cpp** — LRU cache for shaped text runs (font + size + variation axes + string)
- [x] **Atlas page management** — Multiple pages if needed, regenerate on demand
- [x] **Prebuilt atlas loading** — Load .daktfont, use cached glyphs without runtime rasterization

### Text Cursor & Selection

- [x] **TextCursor.cpp** — Render blinking cursor, text selection highlight, caret tracking
- [x] **Animation integration** — Cursor blink via timer, smooth selection animations
- [ ] **Animation integration** — Cursor blink via animator, smooth selection animations

### Tests (Phase 2)

- [ ] **TTF/OTF parsing tests** — Validate common fonts (Arial, Times, DejaVu)
- [ ] **Variable font tests** — Interpolate axes, verify glyph deltas
- [ ] **SDF atlas tests** — Verify distance field correctness, no artifacts
- [ ] **Text shaping tests** — Ligatures, bidi, kerning correctness
- [ ] **Glyph cache tests** — LRU eviction, page management

---

## Phase 3: Vulkan Backend & Shaders ✅

### Draw Command Buffer

> **Note:** Basic DrawList and DrawBatcher moved to Phase 1 and completed.

- [x] **DrawBatcher.cpp enhancements** — Advanced batching: merge by texture/render state, minimize draw calls, clip stack management

### IRenderBackend Interface

- [x] **Backend.hpp designed** — Virtual interface: createResources(), beginFrame(), submit(DrawList), present(), resize(), shutdown()
- [x] **Opaque handles** — VkBuffer → uint64_t, VkImage → uint64_t for ABI stability

### Vulkan Backend Implementation

- [x] **VulkanBackend.hpp** — VkInstance, VkPhysicalDevice, VkDevice, VkSurfaceKHR, swapchain
- [x] **VulkanBackend.cpp** — Initialization, swapchain management, synchronization (fences/semaphores)
- [x] **Resources.cpp** — createBuffer(), createImage(), allocateMemory(), ring allocator for dynamic buffers
- [x] **Rendering.cpp** — beginFrame(), recordCommands(), submit(), present()
- [x] **Descriptor management** — Descriptor sets/pools for texture/sampler binding

### Shaders (Vulkan/GLSL)

- [x] **shaders/sources/ui.glsl** — Vertex/fragment for colored geometry, textured quads
- [x] **shaders/sources/text.glsl** — SDF text rendering with antialiasing
- [x] **Compilation to SPIR-V** — Use glslc or shaderc to compile offline
- [x] **shaders/compiled/ storage** — Pre-compiled SPIR-V modules as hex arrays in C++ headers
- [x] **CMake shader embedding** — Automatic binary → C++ conversion, no runtime compilation

### Vulkan Tests

- [x] **Backend interface tests** — Resource handles, buffer/texture descriptors, capabilities
- [x] **DrawBatcher tests** — Batching, merging, sorting, clip rect handling
- [x] **DrawList integration** — Vertex/index buffers, command generation

---

## Phase 4: Retained Mode & Advanced Widgets ✅

### Immediate Mode API

> **Note:** Basic immediate mode API moved to Phase 1 and completed.

- [x] **Advanced Widgets** — ColorPicker, TextInput with full editing, ComboBox, TreeView, TabBar
- [x] **Menus & Popups** — beginMenu/endMenu, beginPopup/endPopup, context menus
- [x] **Tables** — beginTable/endTable with column configuration, sorting, scrolling

### Retained Mode (UIContainer/Widget Tree)

- [x] **UIContainer.hpp** — Root container holding persistent widget tree, layout root, dirty propagation
- [x] **UIContainer.cpp** — Tree management, add/remove/find widgets, frame processing
- [x] **Widget.hpp** — Base class: state, event handlers, build(Context&) method
- [x] **Widget.cpp** — Concrete widgets: Button, Label, Slider, Checkbox, TextInput, Panel, ScrollView, TreeNode, TabBar
- [x] **Dirty propagation** — Mark ancestors as dirty when child state changes, batch relayout

### Widget Organization (Reorganized)

Widget headers are now organized into subdirectories for better code organization:

- [x] **widgets/WidgetBase.hpp** — Base Widget class and common types (RetainedWidgetFlags, WidgetEvent, etc.)
- [x] **widgets/Label.hpp** — Text label widget
- [x] **widgets/Button.hpp** — Clickable button widget
- [x] **widgets/Checkbox.hpp** — Toggle checkbox widget
- [x] **widgets/RadioButton.hpp** — Radio button with group support
- [x] **widgets/Slider.hpp** — Value slider widget
- [x] **widgets/ProgressBar.hpp** — Progress indicator widget
- [x] **widgets/TextInput.hpp** — Text input field widget
- [x] **widgets/Dropdown.hpp** — Dropdown/ComboBox widget
- [x] **widgets/Panel.hpp** — Container panel widget
- [x] **widgets/ScrollView.hpp** — Scrollable container widget
- [x] **widgets/TreeNode.hpp** — Expandable tree node widget
- [x] **widgets/TabBar.hpp** — Tab bar navigation widget
- [x] **widgets/Shape.hpp** — Geometric shape widgets (Hexagon, Circle, Triangle, Star, Pentagon, Octagon, Diamond, Arrow, Cross)
- [x] **widgets/ColorPicker.hpp** — HSV color picker with RGB/hex input
- [x] **widgets/Menu.hpp** — Menu, MenuBar, ContextMenu, Popup, and Tooltip widgets
- [x] **widgets/Table.hpp** — Data table with sorting, selection, and scrolling
- [x] **Widgets.hpp** — Convenience header that includes all widget types

### Container Organization (Reorganized)

Container headers are now organized into a subdirectory:

- [x] **containers/ContainerBase.hpp** — Base UIContainer class, LayoutDirection, Alignment enums
- [x] **containers/VBox.hpp** — Vertical box layout container
- [x] **containers/HBox.hpp** — Horizontal box layout container
- [x] **containers/Grid.hpp** — Grid layout container with cell spanning
- [x] **containers/Stack.hpp** — Overlay/stack container (z-order layering)
- [x] **containers/Wrap.hpp** — Flow layout with wrapping
- [x] **containers/Splitter.hpp** — Resizable split pane container
- [x] **Containers.hpp** — Convenience header that includes all container types

### Immediate ↔ Retained Bridge

- [x] **Retained Widget build()** — Generate immediate-mode commands, feed to active context
- [ ] **Bidirectional communication** — Immediate API can embed retained subtrees

### Tests (Phase 4)

- [x] **Widget tests** — Widget base class, hierarchy, state management
- [x] **Widget interaction** — Button clicks, slider drags, checkbox toggle, text input
- [x] **Retained tree tests** — Add/remove widgets, dirty propagation correctness
- [x] **UIContainer tests** — Layout, focus management, input routing

---

## Phase 5: C API & C# Bindings

### Animation System

> **Note:** Core animation system (Tween, Spring, Easing) moved to Phase 1 and completed.

- [ ] **Timeline.cpp enhancements** — Compose animations: sequences, parallel, staggering, callbacks on completion
- [ ] **Integration** — Animate layout properties, colors, widget opacity, transitions

### C API (ClangSharp-Compatible)

- [ ] **c_api.h header** — C-safe opaque handles (typedef uint64_t DaktGUI_Context, etc.), no C++ constructs
- [ ] **Context.cpp** — dakt_gui_create(), dakt_gui_destroy(), dakt_gui_new_frame(float delta_time)
- [ ] **Input.cpp** — dakt_gui_process_mouse(int x, int y, uint32_t buttons), dakt_gui_process_key(uint32_t keycode, bool pressed), dakt_gui_process_text(const char* utf8)
- [ ] **Widgets.cpp** — dakt_gui_button(const char* label), dakt_gui_checkbox(const char* label, bool* value), dakt_gui_slider_float(const char* label, float* value, float min, float max), dakt_gui_text_input(const char* id, char* buffer, size_t capacity)
- [ ] **Rendering.cpp** — dakt_gui_render(), dakt_gui_get_backend_type()

### C# Bindings

- [ ] **ClangSharp configuration** — Parse c_api.h, generate P/Invoke interop
- [ ] **DaktLib.GUI.cs generation** — Managed wrappers for all C functions, safe type mapping (IntPtr → nint)
- [ ] **DaktLib.GUI.csproj** — .NET 8+ project, native library loader (platform-specific DLL/SO/dylib)
- [ ] **Managed wrapper classes** — C# Context, Button, Slider classes with RAII pattern

### Tests (Phase 5)

- [ ] **Animation tests** — Easing correctness, timeline sequencing, interpolation
- [ ] **C API tests** — Function ABI stability, opaque handles, memory safety
- [ ] **C# binding tests** — P/Invoke marshaling, managed wrappers, cross-language callbacks

### Showcase Example

- [x] **examples/showcase.cpp** — Demonstrate immediate + retained modes, all widgets, theme switching, animations
- [x] **Platform window** — Create Vulkan surface, input polling, resize handling
- [x] **Shapes demo** — Display various geometric shapes (hexagon, pentagon, star, triangle, etc.)
- [x] **Animated progress bar** — Demonstrate animation with smooth progress transitions

---

## Phase 6: Additional Backends & Release

### Direct3D 11 Backend

- [ ] **DX11Backend.hpp** — ID3D11Device, ID3D11DeviceContext, IDXGISwapChain
- [ ] **DX11Backend.cpp** — Device creation, swapchain management, render target setup
- [ ] **Resources.cpp** — createBuffer(), createTexture(), ID3D11VertexShader/PixelShader creation
- [ ] **Rendering.cpp** — beginFrame(), setRenderTarget(), draw calls, present()
- [ ] **HLSL shaders** — Compile to DXBC/DXIL, bundle in binary

### Direct3D 12 Backend

- [ ] **DX12Backend.hpp** — ID3D12Device, ID3D12CommandQueue, ID3D12CommandList
- [ ] **DX12Backend.cpp** — Descriptor heaps, fence synchronization, swapchain (IDXGISwapChain3)
- [ ] **Resources.cpp** — GPU memory allocation, descriptor creation
- [ ] **Rendering.cpp** — Record command lists, execute, signal/wait fences

### OpenGL 4.5+ Backend

- [ ] **OpenGLBackend.hpp** — GL context, VAO/VBO/FBO management
- [ ] **OpenGLBackend.cpp** — Context creation (platform-specific), extension loading
- [ ] **Resources.cpp** — Create VAOs, VBOs, textures, framebuffers
- [ ] **Rendering.cpp** — Bind VAO, draw instanced, framebuffer operations
- [ ] **GLSL shaders** — GLSL 450 sources, SPIR-V compilation, portable

### Metal Backend

- [ ] **MetalBackend.mm** — Objective-C++: MTLDevice, MTLCommandQueue, CAMetalLayer
- [ ] **MetalBackend.mm** — Device setup, command buffer encoding
- [ ] **Resources.mm** — createBuffer(), createTexture(), MTLRenderPipelineState creation
- [ ] **Rendering.mm** — Encode render commands, commit, present()
- [ ] **MSL shaders** — Metal Shading Language sources, bytecode compilation

### Shader Management Across Platforms

- [ ] **Shader compilation pipeline** — glslc → SPIR-V, fxc/dxc → DXIL, MSL → bytecode
- [ ] **Unified shader sources** — Single GLSL source → multiple formats via compilation tools
- [ ] **Offline compilation** — All shaders pre-compiled in repository (no runtime compilation)
- [ ] **Embedded binaries** — CMake converts binary shaders to hex arrays in headers

### Testing & Validation

- [ ] **Cross-platform build verification** — Compile all backends on respective platforms
- [ ] **Render output tests** — Compare output across backends (golden images)
- [ ] **Performance profiling** — GPU utilization, draw call count, memory usage per backend

### NuGet Package Assembly

- [ ] **Package structure** — DaktLib.GUI/
  - [ ] native/win-x86/, win-x64/, linux-x64/, osx-x64/, osx-arm64/ (static libs + headers)
  - [ ] lib/net8.0/ (C# bindings DLL)
  - [ ] DaktLib.GUI.nuspec metadata (description, dependencies, version)
- [ ] **CMake packaging script** — Gather artifacts, create .nupkg with correct RIDs
- [ ] **Local package testing** — Consume NuGet in sample C# project, verify native lib loading

### Release Automation

- [ ] **GitHub Actions release job** — Trigger on version tag (v0.1.0), build all platforms, create assets
- [ ] **NuGet publish** — Push DaktLib.GUI.nupkg to nuget.org with API key
- [ ] **GitHub Releases** — Create release notes, attach standalone artifacts
- [ ] **Changelog generation** — Auto-generate from git commit messages

### Documentation

- [ ] **GETTING_STARTED.md** — Installation (NuGet), minimal example (Vulkan window)
- [ ] **API_REFERENCE.md** — Immediate API functions, Widget types, Theme customization
- [ ] **ARCHITECTURE_DEEP_DIVE.md** — Layout algorithm, draw batching, font atlas format
- [ ] **BACKEND_SELECTION.md** — Platform availability, performance characteristics, requirements
- [ ] **FONT_GENERATION.md** — Loading TTF/OTF, creating .daktfont atlases, variable font usage
- [ ] **C_API.md** — C function reference, opaque handles, calling conventions
- [ ] **C#_BINDINGS.md** — P/Invoke usage, managed wrapper classes, platform native lib loading
- [ ] **CONTRIBUTING.md** — Build instructions, test procedures, pull request guidelines

---

## Summary

**Total Tasks: ~56 tracked items across 7 phases**

- **Phase 0**: 15 tasks (scaffold, build, CI/CD, packaging)
- **Phase 1**: ✅ COMPLETE — Core types, layout, styling, input, animation, immediate mode, draw commands, tests
- **Phase 2**: ✅ COMPLETE — TTF/OTF parsing, variable fonts, SDF/MSDF generation, text shaping, glyph caching, text cursor
- **Phase 3**: ✅ COMPLETE — 12 tasks (Vulkan backend, shaders)
- **Phase 4**: 8 tasks (retained mode, advanced widgets)
- **Phase 5**: 10 tasks (C API, C# bindings)
- **Phase 6**: 20+ tasks (additional backends, packaging, docs, testing)

**Release Artifacts per Platform:**
- Static library (libDaktLib-GUI.a / DaktLib-GUI.lib)
- Headers (dakt/gui/*.hpp)
- C# bindings (DaktLib.GUI.cs / .dll per platform)

**Single NuGet Package:** `DaktLib.GUI` with embedded natives for all platforms.
