#include "dakt/gui/core/Types.hpp"
#include <chrono>
#include <random>

namespace dakt::gui {

ID generateID(uint64_t value) {
    static std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    return rng();
}

float Vec2::length() const { return std::sqrt(x * x + y * y); }

Color Color::fromHSV(float h, float s, float v) {
    // Convert HSV to RGB
    // h: 0-360, s: 0-1, v: 0-1
    float c = v * s;
    float x = c * (1 - std::fmod(h / 60.0f, 2) + 1);
    float m = v - c;

    float r, g, b;
    if (h < 60) {
        r = c;
        g = x;
        b = 0;
    } else if (h < 120) {
        r = x;
        g = c;
        b = 0;
    } else if (h < 180) {
        r = 0;
        g = c;
        b = x;
    } else if (h < 240) {
        r = 0;
        g = x;
        b = c;
    } else if (h < 300) {
        r = x;
        g = 0;
        b = c;
    } else {
        r = c;
        g = 0;
        b = x;
    }

    return Color(static_cast<uint8_t>((r + m) * 255), static_cast<uint8_t>((g + m) * 255), static_cast<uint8_t>((b + m) * 255));
}

bool Rect::contains(const Vec2& point) const { return point.x >= x && point.x <= x + width && point.y >= y && point.y <= y + height; }

bool Rect::intersects(const Rect& other) const { return !(right() < other.x || x > other.right() || bottom() < other.y || y > other.bottom()); }

Rect Rect::intersection(const Rect& other) const {
    float ix = std::max(x, other.x);
    float iy = std::max(y, other.y);
    float iw = std::min(right(), other.right()) - ix;
    float ih = std::min(bottom(), other.bottom()) - iy;

    if (iw < 0 || ih < 0)
        return Rect();
    return Rect(ix, iy, iw, ih);
}

} // namespace dakt::gui
