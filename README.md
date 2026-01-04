# DaktLib-GUI

Custom immediate-first GUI with optional retained containers and five render backends (DX11/12, Vulkan, OpenGL, Metal). Dependency-free C++23 with a C API friendly to ClangSharp.

## Layout
- Public headers: `include/dakt/gui` (subfolders per subsystem: core, immediate, retained, layout, style, input, animation, draw, text, backend, c_api).
- Sources: `src/` mirrors the same structure.
- Examples: `examples/` (showcase stub).
- Tests: `tests/` (smoke test to validate headers and basic wiring).
- Shaders: `shaders/` (place HLSL/GLSL/MSL sources).

## Building
```bash
cmake -S . -B build -G "Ninja" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=ON \
  -DDAKT_GUI_BUILD_TESTS=ON \
  -DDAKT_GUI_BUILD_SHOWCASE=ON
cmake --build build
```

Notes:
- Requires a CMake >= 4.2.1 generator and a C++23 compiler.
- Windows builds add DX11/DX12 backends; macOS builds add Metal (as Objective-C++). Linux/macOS use Vulkan/OpenGL sources by default.

## Testing
```bash
ctest --test-dir build
```
`tests/smoke.cpp` exercises the basic API surface and backend plumbing with a dummy backend.

## Showcase
The stub in `examples/showcase.cpp` wires a no-op backend to the public API. Extend it with real widget calls as immediate/retained APIs land.

## Roadmap
See `TODO.md` for phased tasks and milestones. The modular folder layout keeps each subsystem isolated for incremental development and backend-specific toggling.
