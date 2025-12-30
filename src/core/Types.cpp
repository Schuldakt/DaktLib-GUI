// ============================================================================
// DaktLib GUI Module - Types Implementation
// ============================================================================

#include <dakt/gui/core/Types.hpp>

#include <cmath>
#include <numbers>

namespace dakt::gui
{

// ============================================================================
// Color
// ============================================================================

Color Color::fromHSV(f32 h, f32 s, f32 v, f32 a)
{
    // Normalize hue to [0, 360)
    h = std::fmod(h, 360.0f);
    if (h < 0.0f)
        h += 360.0f;

    s = std::clamp(s, 0.0f, 1.0f);
    v = std::clamp(v, 0.0f, 1.0f);

    f32 c = v * s;
    f32 x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    f32 m = v - c;

    f32 r, g, b;

    if (h < 60.0f)
    {
        r = c;
        g = x;
        b = 0;
    }
    else if (h < 120.0f)
    {
        r = x;
        g = c;
        b = 0;
    }
    else if (h < 180.0f)
    {
        r = 0;
        g = c;
        b = x;
    }
    else if (h < 240.0f)
    {
        r = 0;
        g = x;
        b = c;
    }
    else if (h < 300.0f)
    {
        r = x;
        g = 0;
        b = c;
    }
    else
    {
        r = c;
        g = 0;
        b = x;
    }

    return Color::fromFloat(r + m, g + m, b + m, a);
}

// ============================================================================
// Triangle
// ============================================================================

bool Triangle::contains(Vec2 p) const
{
    // Barycentric coordinate method
    auto sign = [](Vec2 p1, Vec2 p2, Vec2 p3) { return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y); };

    f32 d1 = sign(p, a, b);
    f32 d2 = sign(p, b, c);
    f32 d3 = sign(p, c, a);

    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(hasNeg && hasPos);
}

// ============================================================================
// Hexagon
// ============================================================================

std::array<Vec2, 6> Hexagon::vertices() const
{
    std::array<Vec2, 6> verts;

    f32 angleOffset = flat ? 0.0f : (std::numbers::pi_v<f32> / 6.0f);

    for (usize i = 0; i < 6; ++i)
    {
        f32 angle = angleOffset + (std::numbers::pi_v<f32> / 3.0f) * static_cast<f32>(i);
        verts[i] = Vec2{center.x + radius * std::cos(angle), center.y + radius * std::sin(angle)};
    }

    return verts;
}

bool Hexagon::contains(Vec2 p) const
{
    // Check if point is inside hexagon using cross product method
    auto verts = vertices();

    for (usize i = 0; i < 6; ++i)
    {
        Vec2 v0 = verts[i];
        Vec2 v1 = verts[(i + 1) % 6];

        Vec2 edge = v1 - v0;
        Vec2 toPoint = p - v0;

        // Check if point is on the left side of each edge
        if (edge.cross(toPoint) < 0)
        {
            return false;
        }
    }

    return true;
}

Rect Hexagon::bounds() const
{
    auto verts = vertices();

    Vec2 minV = verts[0];
    Vec2 maxV = verts[0];

    for (usize i = 1; i < 6; ++i)
    {
        minV = Vec2::min(minV, verts[i]);
        maxV = Vec2::max(maxV, verts[i]);
    }

    return Rect{minV, maxV};
}

}  // namespace dakt::gui
