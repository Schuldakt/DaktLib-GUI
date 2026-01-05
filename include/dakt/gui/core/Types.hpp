#ifndef DAKT_GUI_TYPES_HPP
#define DAKT_GUI_TYPES_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>


namespace dakt::gui {

// ============================================================================
// Primitive Types & Utilities
// ============================================================================

using ID = uint64_t;

// Generate unique ID from 64-bit value
ID generateID(uint64_t value);

// ============================================================================
// Vectors & Math
// ============================================================================

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }

    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }

    Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }

    float dot(const Vec2& other) const { return x * other.x + y * other.y; }

    float length() const;
};

struct Vec2i {
    int32_t x = 0;
    int32_t y = 0;

    Vec2i() = default;
    Vec2i(int32_t x, int32_t y) : x(x), y(y) {}
};

// ============================================================================
// Colors
// ============================================================================

struct Color {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;

    Color() = default;
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}

    explicit Color(uint32_t rgba) {
        r = (rgba >> 24) & 0xFF;
        g = (rgba >> 16) & 0xFF;
        b = (rgba >> 8) & 0xFF;
        a = rgba & 0xFF;
    }

    uint32_t toRGBA() const { return (static_cast<uint32_t>(r) << 24) | (static_cast<uint32_t>(g) << 16) | (static_cast<uint32_t>(b) << 8) | static_cast<uint32_t>(a); }

    static Color fromHSV(float h, float s, float v);
};

// ============================================================================
// Rectangles & Geometry
// ============================================================================

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    Rect() = default;
    Rect(float x, float y, float width, float height) : x(x), y(y), width(width), height(height) {}

    float right() const { return x + width; }
    float bottom() const { return y + height; }

    Vec2 topLeft() const { return Vec2(x, y); }
    Vec2 center() const { return Vec2(x + width * 0.5f, y + height * 0.5f); }
    Vec2 size() const { return Vec2(width, height); }

    bool contains(const Vec2& point) const;
    bool intersects(const Rect& other) const;
    Rect intersection(const Rect& other) const;
};

// ============================================================================
// Alignment & Layout
// ============================================================================

enum class Align { None = 0, Left = 1, Right = 2, Top = 4, Bottom = 8, HCenter = 16, VCenter = 32, Center = HCenter | VCenter };

enum class FlexDirection { Row, Column };

enum class JustifyContent { FlexStart, FlexEnd, Center, SpaceBetween, SpaceAround, SpaceEvenly };

enum class AlignItems { FlexStart, FlexEnd, Center, Stretch, Baseline };

enum class FlexWrap { NoWrap, Wrap, WrapReverse };

// ============================================================================
// Input Types
// ============================================================================

enum class MouseButton { Left = 0, Right = 1, Middle = 2, Back = 3, Forward = 4 };

enum class Key {
    None = 0,
    A = 1,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    Num0 = 27,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    F1 = 37,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    Escape,
    Tab,
    CapsLock,
    Shift,
    Control,
    Alt,
    Space,
    Enter,
    Backspace,
    Delete,
    Home,
    End,
    PageUp,
    PageDown,
    ArrowUp,
    ArrowDown,
    ArrowLeft,
    ArrowRight,
    Insert,
    PrintScreen,
    Pause,
    NumPad0 = 70,
    NumPad1,
    NumPad2,
    NumPad3,
    NumPad4,
    NumPad5,
    NumPad6,
    NumPad7,
    NumPad8,
    NumPad9,
    NumPadMultiply,
    NumPadAdd,
    NumPadMinus,
    NumPadDecimal,
    NumPadDivide
};

struct MouseInput {
    Vec2 position;
    Vec2 delta;
    float wheelDelta = 0.0f;
    bool buttons[5] = {};
    bool prevButtons[5] = {};
};

struct KeyboardInput {
    Key keys[256] = {};
    bool keyPressed[256] = {};
    bool keyReleased[256] = {};
    std::string textInput;
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
};

// ============================================================================
// Styling Types
// ============================================================================

enum class WidgetState { Normal, Hover, Active, Disabled, Focus };

struct EdgeInsets {
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
    float left = 0.0f;

    EdgeInsets() = default;
    EdgeInsets(float all) : top(all), right(all), bottom(all), left(all) {}
    EdgeInsets(float vertical, float horizontal) : top(vertical), right(horizontal), bottom(vertical), left(horizontal) {}
    EdgeInsets(float top, float right, float bottom, float left) : top(top), right(right), bottom(bottom), left(left) {}
};

struct BorderRadius {
    float topLeft = 0.0f;
    float topRight = 0.0f;
    float bottomRight = 0.0f;
    float bottomLeft = 0.0f;

    BorderRadius() = default;
    explicit BorderRadius(float all) : topLeft(all), topRight(all), bottomRight(all), bottomLeft(all) {}
};

struct Shadow {
    Vec2 offset;
    float blur = 0.0f;
    float spread = 0.0f;
    Color color = Color(0, 0, 0, 128);
};

} // namespace dakt::gui

#endif // DAKT_GUI_TYPES_HPP
