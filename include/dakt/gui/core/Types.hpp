// ============================================================================
// DaktLib GUI Module - Core Types
// ============================================================================
// GUI-specific types. Basic geometry types are provided by Core module.
// ============================================================================

#pragma once

#include <dakt/core/Geometry.hpp>
#include <dakt/core/String.hpp>
#include <dakt/core/Types.hpp>

#include <algorithm>
#include <array>
#include <cmath>

namespace dakt::gui
{

using namespace dakt::core;

// Re-export core geometry types into gui namespace for convenience
using Vec2 = dakt::core::Vec2;
using Vec4 = dakt::core::Vec4;
using Color = dakt::core::Color;
using Rect = dakt::core::Rect;
using Circle = dakt::core::Circle;

// ============================================================================
// GUI-specific Shape Types
// ============================================================================

struct Triangle
{
    Vec2 a, b, c;

    constexpr Triangle() = default;
    constexpr Triangle(Vec2 a_, Vec2 b_, Vec2 c_) : a(a_), b(b_), c(c_) {}

    [[nodiscard]] bool contains(Vec2 p) const;
    [[nodiscard]] constexpr Rect bounds() const
    {
        return Rect{Vec2::min(Vec2::min(a, b), c), Vec2::max(Vec2::max(a, b), c)};
    }
};

struct Hexagon
{
    Vec2 center;
    f32 radius = 0.0f;
    bool flat = true;  // true = flat-top, false = pointy-top

    constexpr Hexagon() = default;
    constexpr Hexagon(Vec2 c, f32 r, bool isFlat = true) : center(c), radius(r), flat(isFlat) {}

    [[nodiscard]] std::array<Vec2, 6> vertices() const;
    [[nodiscard]] bool contains(Vec2 p) const;
    [[nodiscard]] Rect bounds() const;
};

// ============================================================================
// Edge/Corner Types
// ============================================================================

enum class Corner : u8
{
    None = 0,
    TopLeft = 1 << 0,
    TopRight = 1 << 1,
    BottomLeft = 1 << 2,
    BottomRight = 1 << 3,
    Top = TopLeft | TopRight,
    Bottom = BottomLeft | BottomRight,
    Left = TopLeft | BottomLeft,
    Right = TopRight | BottomRight,
    All = TopLeft | TopRight | BottomLeft | BottomRight
};

inline constexpr Corner operator|(Corner a, Corner b)
{
    return static_cast<Corner>(static_cast<u8>(a) | static_cast<u8>(b));
}

inline constexpr Corner operator&(Corner a, Corner b)
{
    return static_cast<Corner>(static_cast<u8>(a) | static_cast<u8>(b));
}

inline constexpr bool hasFlag(Corner flags, Corner flag)
{
    return (static_cast<u8>(flags) & static_cast<u8>(flag)) != 0;
}

// ============================================================================
// Text Alignment
// ============================================================================

enum class HAlign : u8
{
    Left,
    Center,
    Right
};

enum class VAlign : u8
{
    Top,
    Middle,
    Bottom
};

struct TextAlign
{
    HAlign h = HAlign::Left;
    VAlign v = VAlign::Top;

    static constexpr TextAlign topLeft() { return {HAlign::Left, VAlign::Top}; }
    static constexpr TextAlign topCenter() { return {HAlign::Center, VAlign::Top}; }
    static constexpr TextAlign topRight() { return {HAlign::Right, VAlign::Top}; }
    static constexpr TextAlign centerLeft() { return {HAlign::Left, VAlign::Middle}; }
    static constexpr TextAlign center() { return {HAlign::Center, VAlign::Middle}; }
    static constexpr TextAlign centerRight() { return {HAlign::Right, VAlign::Middle}; }
    static constexpr TextAlign bottomLeft() { return {HAlign::Left, VAlign::Bottom}; }
    static constexpr TextAlign bottomCenter() { return {HAlign::Center, VAlign::Bottom}; }
    static constexpr TextAlign bottomRight() { return {HAlign::Right, VAlign::Bottom}; }
};

// ============================================================================
// Cursor Types
// ============================================================================

enum class Cursor : u8
{
    Arrow,
    IBeam,
    Wait,
    Cross,
    Hand,
    ResizeNS,
    ResizeEW,
    ResizeNWSE,
    ResizeNESW,
    ResizeAll,
    NotAllowed,
    Hidden
};

// ============================================================================
// Input Types
// ============================================================================

enum class MouseButton : u8
{
    Left = 0,
    Right = 1,
    Middle = 2,
    X1 = 3,
    X2 = 4,
    Count = 5
};

enum class Key : u16
{
    None = 0,

    // Letters
    A = 'A',
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
    Z = 'Z',

    // Numbers
    Num0 = '0',
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9 = '9',

    // Function keys
    F1 = 256,
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

    // Special keys
    Escape,
    Tab,
    Space,
    Enter,
    Backspace,
    Delete,
    Insert,
    Home,
    End,
    PageUp,
    PageDown,
    Left,
    Right,
    Up,
    Down,

    // Modifiers
    LeftShift,
    RightShift,
    LeftCtrl,
    RightCtrl,
    LeftAlt,
    RightAlt,

    // Numpad
    Numpad0,
    Numpad1,
    Numpad2,
    Numpad3,
    Numpad4,
    Numpad5,
    Numpad6,
    Numpad7,
    Numpad8,
    Numpad9,
    NumpadAdd,
    NumpadSubtract,
    NumpadMultiply,
    NumpadDivide,
    NumpadDecimal,
    NumpadEnter,

    // Punctuation
    Comma,
    Period,
    Slash,
    Semicolon,
    Quote,
    LeftBracket,
    RightBracket,
    Backslash,
    Grave,
    Minus,
    Equals,

    Count
};

struct KeyMods
{
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
    bool super = false;  // Windows/Command key

    [[nodiscard]] constexpr bool none() const { return !shift && !ctrl && !alt && !super; }

    [[nodiscard]] constexpr bool operator==(KeyMods other) const
    {
        return shift == other.shift && ctrl == other.ctrl && alt == other.alt && super == other.super;
    }
};

}  // namespace dakt::gui
