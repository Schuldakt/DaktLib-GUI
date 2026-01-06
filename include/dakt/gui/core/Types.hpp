#ifndef DAKTLIB_GUI_TYPES_HPP
#define DAKTLIB_GUI_TYPES_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

// ============================================================================
// Export Macros
// ============================================================================

#if defined(DAKTLIB_GUI_STATIC)
#define DAKTLIB_GUI_API
#elif defined(DAKTLIB_GUI_EXPORTS)
#if defined(_WIN32)
#define DAKTLIB_GUI_API __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define DAKTLIB_GUI_API __attribute__((visibility("default")))
#else
#define DAKTLIB_GUI_API
#endif
#else
#if defined(_WIN32)
#define DAKTLIB_GUI_API __declspec(dllimport)
#else
#define DAKTLIB_GUI_API
#endif
#endif

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

struct DAKTLIB_GUI_API Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
    Vec2 operator/(float scalar) const { return Vec2(x / scalar, y / scalar); }
    Vec2& operator+=(const Vec2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
    Vec2& operator-=(const Vec2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    Vec2& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }
    bool operator==(const Vec2& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vec2& other) const { return !(*this == other); }

    float dot(const Vec2& other) const { return x * other.x + y * other.y; }
    float cross(const Vec2& other) const { return x * other.y - y * other.x; }
    float length() const;
    float lengthSquared() const { return x * x + y * y; }
    Vec2 normalized() const;
    Vec2 perpendicular() const { return Vec2(-y, x); }

    static Vec2 lerp(const Vec2& a, const Vec2& b, float t) { return Vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); }
};

struct DAKTLIB_GUI_API Vec2i {
    int32_t x = 0;
    int32_t y = 0;

    Vec2i() = default;
    Vec2i(int32_t x, int32_t y) : x(x), y(y) {}

    Vec2i operator+(const Vec2i& other) const { return Vec2i(x + other.x, y + other.y); }
    Vec2i operator-(const Vec2i& other) const { return Vec2i(x - other.x, y - other.y); }
    bool operator==(const Vec2i& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vec2i& other) const { return !(*this == other); }

    Vec2 toFloat() const { return Vec2(static_cast<float>(x), static_cast<float>(y)); }
};

struct DAKTLIB_GUI_API Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3() = default;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
    Vec3 operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
    Vec3 operator*(float scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
    Vec3 operator/(float scalar) const { return Vec3(x / scalar, y / scalar, z / scalar); }

    float& operator[](int i) { return (&x)[i]; }
    const float& operator[](int i) const { return (&x)[i]; }

    float dot(const Vec3& other) const { return x * other.x + y * other.y + z * other.z; }
    Vec3 cross(const Vec3& other) const { return Vec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x); }
    float length() const;
    float lengthSquared() const { return x * x + y * y + z * z; }
    Vec3 normalized() const;
};

// ============================================================================
// Colors
// ============================================================================

struct DAKTLIB_GUI_API Color {
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

    bool operator==(const Color& other) const { return r == other.r && g == other.g && b == other.b && a == other.a; }
    bool operator!=(const Color& other) const { return !(*this == other); }

    uint32_t toRGBA() const { return (static_cast<uint32_t>(r) << 24) | (static_cast<uint32_t>(g) << 16) | (static_cast<uint32_t>(b) << 8) | static_cast<uint32_t>(a); }

    uint32_t toABGR() const { return (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(r); }

    Color withAlpha(uint8_t newAlpha) const { return Color(r, g, b, newAlpha); }

    static Color fromHSV(float h, float s, float v);
    static Color lerp(const Color& a, const Color& b, float t);

    // Common colors
    static Color white() { return Color(255, 255, 255, 255); }
    static Color black() { return Color(0, 0, 0, 255); }
    static Color transparent() { return Color(0, 0, 0, 0); }
    static Color red() { return Color(255, 0, 0, 255); }
    static Color green() { return Color(0, 255, 0, 255); }
    static Color blue() { return Color(0, 0, 255, 255); }
};

// ============================================================================
// Rectangles & Geometry
// ============================================================================

// Forward declaration
struct EdgeInsets;

struct DAKTLIB_GUI_API Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    Rect() = default;
    Rect(float x, float y, float width, float height) : x(x), y(y), width(width), height(height) {}
    Rect(const Vec2& pos, const Vec2& size) : x(pos.x), y(pos.y), width(size.x), height(size.y) {}

    float right() const { return x + width; }
    float bottom() const { return y + height; }

    Vec2 topLeft() const { return Vec2(x, y); }
    Vec2 topRight() const { return Vec2(x + width, y); }
    Vec2 bottomLeft() const { return Vec2(x, y + height); }
    Vec2 bottomRight() const { return Vec2(x + width, y + height); }
    Vec2 center() const { return Vec2(x + width * 0.5f, y + height * 0.5f); }
    Vec2 size() const { return Vec2(width, height); }

    bool contains(const Vec2& point) const;
    bool intersects(const Rect& other) const;
    Rect intersection(const Rect& other) const;
    Rect unionWith(const Rect& other) const;

    Rect expanded(float amount) const { return Rect(x - amount, y - amount, width + amount * 2, height + amount * 2); }
    Rect expanded(const EdgeInsets& insets) const;
    Rect contracted(float amount) const { return expanded(-amount); }
    Rect contracted(const EdgeInsets& insets) const;

    bool operator==(const Rect& other) const { return x == other.x && y == other.y && width == other.width && height == other.height; }
    bool operator!=(const Rect& other) const { return !(*this == other); }
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
    BorderRadius(float tl, float tr, float br, float bl) : topLeft(tl), topRight(tr), bottomRight(br), bottomLeft(bl) {}
};

struct Shadow {
    Vec2 offset;
    float blur = 0.0f;
    float spread = 0.0f;
    Color color = Color(0, 0, 0, 128);
};

} // namespace dakt::gui

#endif // DAKTLIB_GUI_TYPES_HPP
