#include "dakt/gui/core/Types.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>

namespace dakt::gui {

// ============================================================================
// ID Generation
// ============================================================================

static std::mt19937_64& getIDGenerator() {
    static std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    return rng;
}

ID generateID(uint64_t value) {
    // FNV-1a hash combined with random state for uniqueness
    constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;
    constexpr uint64_t FNV_PRIME = 1099511628211ULL;

    uint64_t hash = FNV_OFFSET;
    hash ^= value;
    hash *= FNV_PRIME;
    hash ^= getIDGenerator()();

    return hash;
}

// ============================================================================
// Vec2 Implementation
// ============================================================================

float Vec2::length() const { return std::sqrt(x * x + y * y); }

Vec2 Vec2::normalized() const {
    float len = length();
    if (len > 0.0f) {
        return Vec2(x / len, y / len);
    }
    return Vec2(0.0f, 0.0f);
}

// ============================================================================
// Vec3 Implementation
// ============================================================================

float Vec3::length() const { return std::sqrt(x * x + y * y + z * z); }

Vec3 Vec3::normalized() const {
    float len = length();
    if (len > 0.0f) {
        return Vec3(x / len, y / len, z / len);
    }
    return Vec3(0.0f, 0.0f, 0.0f);
}

// ============================================================================
// Color Implementation
// ============================================================================

Color Color::fromHSV(float h, float s, float v) {
    // Normalize h to [0, 360)
    h = std::fmod(h, 360.0f);
    if (h < 0.0f)
        h += 360.0f;

    float c = v * s;
    float hPrime = h / 60.0f;
    float x = c * (1.0f - std::fabs(std::fmod(hPrime, 2.0f) - 1.0f));
    float m = v - c;

    float r, g, b;
    if (hPrime < 1.0f) {
        r = c;
        g = x;
        b = 0.0f;
    } else if (hPrime < 2.0f) {
        r = x;
        g = c;
        b = 0.0f;
    } else if (hPrime < 3.0f) {
        r = 0.0f;
        g = c;
        b = x;
    } else if (hPrime < 4.0f) {
        r = 0.0f;
        g = x;
        b = c;
    } else if (hPrime < 5.0f) {
        r = x;
        g = 0.0f;
        b = c;
    } else {
        r = c;
        g = 0.0f;
        b = x;
    }

    return Color(static_cast<uint8_t>((r + m) * 255.0f), static_cast<uint8_t>((g + m) * 255.0f), static_cast<uint8_t>((b + m) * 255.0f), 255);
}

Color Color::lerp(const Color& a, const Color& b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return Color(static_cast<uint8_t>(a.r + (b.r - a.r) * t), static_cast<uint8_t>(a.g + (b.g - a.g) * t), static_cast<uint8_t>(a.b + (b.b - a.b) * t), static_cast<uint8_t>(a.a + (b.a - a.a) * t));
}

// ============================================================================
// Rect Implementation
// ============================================================================

bool Rect::contains(const Vec2& point) const { return point.x >= x && point.x <= x + width && point.y >= y && point.y <= y + height; }

bool Rect::intersects(const Rect& other) const { return !(right() < other.x || x > other.right() || bottom() < other.y || y > other.bottom()); }

Rect Rect::intersection(const Rect& other) const {
    float ix = std::max(x, other.x);
    float iy = std::max(y, other.y);
    float iw = std::min(right(), other.right()) - ix;
    float ih = std::min(bottom(), other.bottom()) - iy;

    if (iw < 0.0f || ih < 0.0f) {
        return Rect();
    }
    return Rect(ix, iy, iw, ih);
}

Rect Rect::unionWith(const Rect& other) const {
    if (width <= 0.0f || height <= 0.0f)
        return other;
    if (other.width <= 0.0f || other.height <= 0.0f)
        return *this;

    float minX = std::min(x, other.x);
    float minY = std::min(y, other.y);
    float maxX = std::max(right(), other.right());
    float maxY = std::max(bottom(), other.bottom());

    return Rect(minX, minY, maxX - minX, maxY - minY);
}

Rect Rect::expanded(const EdgeInsets& insets) const { return Rect(x - insets.left, y - insets.top, width + insets.left + insets.right, height + insets.top + insets.bottom); }

Rect Rect::contracted(const EdgeInsets& insets) const { return Rect(x + insets.left, y + insets.top, std::max(0.0f, width - insets.left - insets.right), std::max(0.0f, height - insets.top - insets.bottom)); }

} // namespace dakt::gui
