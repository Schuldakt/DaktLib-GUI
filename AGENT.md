# AGENT Brief — DaktLib-GUI

## Mission
- Custom immediate-first GUI with optional retained tree and multi-backend rendering (DX11/12, Vulkan, OpenGL, Metal).

## Constraints
- C++23, dependency-free; cross-platform.
- No Dear ImGui or external GUI libs; all widgets/layout/animation in-house.
- C API kept ClangSharp-friendly.

## Scope Highlights
- Flex-inspired layout, styling/themes, animation, SDF/MSDF text shaping.
- DrawList batching and backend abstraction for five renderers.
- Optional retained widgets for complex views; accessibility hooks planned.

## Limitations
- No third-party font rasterizers; use internal SDF/MSDF pipeline.
- Avoid OS-private APIs; rely on app-provided swapchains/surfaces.
