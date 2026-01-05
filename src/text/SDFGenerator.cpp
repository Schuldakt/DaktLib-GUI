#include "dakt/gui/text/SDFGenerator.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace dakt::gui {

SDFGenerator::SDFGenerator() = default;
SDFGenerator::~SDFGenerator() = default;

// ============================================================================
// Shape Construction
// ============================================================================

Shape SDFGenerator::outlineToShape(const GlyphOutline& outline, float scale) {
    Shape shape;

    for (const auto& glyphContour : outline.contours) {
        if (glyphContour.points.empty())
            continue;

        Contour contour;
        const auto& points = glyphContour.points;
        size_t n = points.size();

        // Process points into edge segments
        // TrueType uses quadratic beziers with on-curve and off-curve points
        size_t i = 0;
        while (i < n) {
            // Find starting on-curve point
            size_t startIdx = i;
            while (!points[startIdx % n].onCurve && startIdx < i + n) {
                ++startIdx;
            }
            if (startIdx >= i + n) {
                // No on-curve points, create implied midpoint
                Vec2 p0((points[i].x + points[(i + 1) % n].x) * 0.5f * scale, (points[i].y + points[(i + 1) % n].y) * 0.5f * scale);
                startIdx = i;
            }

            Vec2 currentPoint(points[startIdx % n].x * scale, points[startIdx % n].y * scale);

            for (size_t j = 1; j <= n; ++j) {
                size_t idx = (startIdx + j) % n;
                const auto& pt = points[idx];
                Vec2 nextPoint(pt.x * scale, pt.y * scale);

                if (pt.onCurve) {
                    // Linear segment
                    EdgeSegment edge;
                    edge.type = EdgeSegment::Linear;
                    edge.p0 = currentPoint;
                    edge.p1 = nextPoint;
                    contour.edges.push_back(edge);
                    currentPoint = nextPoint;
                } else {
                    // Off-curve point - quadratic bezier
                    // Look ahead for next point
                    size_t nextIdx = (idx + 1) % n;
                    const auto& nextPt = points[nextIdx];

                    Vec2 controlPoint = nextPoint;
                    Vec2 endPoint;

                    if (nextPt.onCurve) {
                        endPoint = Vec2(nextPt.x * scale, nextPt.y * scale);
                        ++j; // Skip the end point in outer loop
                    } else {
                        // Implied on-curve point between two off-curve points
                        endPoint = (controlPoint + Vec2(nextPt.x * scale, nextPt.y * scale)) * 0.5f;
                    }

                    EdgeSegment edge;
                    edge.type = EdgeSegment::Quadratic;
                    edge.p0 = currentPoint;
                    edge.p1 = controlPoint;
                    edge.p2 = endPoint;
                    contour.edges.push_back(edge);
                    currentPoint = endPoint;
                }
            }

            break; // Only process once per contour
        }

        // Calculate winding direction
        float area = 0;
        for (const auto& edge : contour.edges) {
            area += (edge.p1.x - edge.p0.x) * (edge.p1.y + edge.p0.y);
        }
        contour.winding = (area > 0) ? 1 : -1;

        if (!contour.edges.empty()) {
            shape.contours.push_back(contour);
        }
    }

    return shape;
}

// ============================================================================
// Distance Calculations
// ============================================================================

float SDFGenerator::distanceToLine(Vec2 p, Vec2 a, Vec2 b) const {
    Vec2 ab = b - a;
    Vec2 ap = p - a;

    float t = ab.dot(ap) / ab.dot(ab);
    t = std::clamp(t, 0.0f, 1.0f);

    Vec2 closest = a + ab * t;
    return (p - closest).length();
}

float SDFGenerator::distanceToQuadratic(Vec2 p, Vec2 p0, Vec2 p1, Vec2 p2) const {
    // Solve for closest point on quadratic bezier
    // B(t) = (1-t)²p0 + 2(1-t)t·p1 + t²p2

    // Numerical approach: sample curve and find minimum
    float minDist = std::numeric_limits<float>::max();
    constexpr int SAMPLES = 16;

    for (int i = 0; i <= SAMPLES; ++i) {
        float t = static_cast<float>(i) / SAMPLES;
        float u = 1.0f - t;

        Vec2 point = p0 * (u * u) + p1 * (2 * u * t) + p2 * (t * t);
        float dist = (p - point).length();
        minDist = std::min(minDist, dist);
    }

    // Refine with Newton-Raphson (optional, for higher precision)
    // For now, the sampling is sufficient for SDF generation

    return minDist;
}

float SDFGenerator::distanceToCubic(Vec2 p, Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3) const {
    // Sample cubic bezier curve
    float minDist = std::numeric_limits<float>::max();
    constexpr int SAMPLES = 24;

    for (int i = 0; i <= SAMPLES; ++i) {
        float t = static_cast<float>(i) / SAMPLES;
        float u = 1.0f - t;

        Vec2 point = p0 * (u * u * u) + p1 * (3 * u * u * t) + p2 * (3 * u * t * t) + p3 * (t * t * t);
        float dist = (p - point).length();
        minDist = std::min(minDist, dist);
    }

    return minDist;
}

float SDFGenerator::distanceToEdge(const EdgeSegment& edge, Vec2 point) const {
    switch (edge.type) {
    case EdgeSegment::Linear:
        return distanceToLine(point, edge.p0, edge.p1);
    case EdgeSegment::Quadratic:
        return distanceToQuadratic(point, edge.p0, edge.p1, edge.p2);
    case EdgeSegment::Cubic:
        return distanceToCubic(point, edge.p0, edge.p1, edge.p2, edge.p3);
    }
    return std::numeric_limits<float>::max();
}

int SDFGenerator::calculateWinding(const Contour& contour, Vec2 point) const {
    // Ray casting algorithm for winding number
    int winding = 0;

    for (const auto& edge : contour.edges) {
        Vec2 p0 = edge.p0;
        Vec2 p1 = (edge.type == EdgeSegment::Linear) ? edge.p1 : edge.p2;

        if (p0.y <= point.y) {
            if (p1.y > point.y) {
                // Upward crossing
                float cross = (p1.x - p0.x) * (point.y - p0.y) - (point.x - p0.x) * (p1.y - p0.y);
                if (cross > 0)
                    ++winding;
            }
        } else {
            if (p1.y <= point.y) {
                // Downward crossing
                float cross = (p1.x - p0.x) * (point.y - p0.y) - (point.x - p0.x) * (p1.y - p0.y);
                if (cross < 0)
                    --winding;
            }
        }
    }

    return winding;
}

float SDFGenerator::signedDistance(const Shape& shape, Vec2 point) const {
    float minDist = std::numeric_limits<float>::max();

    // Find minimum distance to any edge
    for (const auto& contour : shape.contours) {
        for (const auto& edge : contour.edges) {
            float dist = distanceToEdge(edge, point);
            minDist = std::min(minDist, dist);
        }
    }

    // Determine sign using winding number
    int totalWinding = 0;
    for (const auto& contour : shape.contours) {
        totalWinding += calculateWinding(contour, point);
    }

    // Inside if winding != 0 (non-zero fill rule)
    return (totalWinding != 0) ? -minDist : minDist;
}

// ============================================================================
// MSDF Edge Coloring
// ============================================================================

void SDFGenerator::colorEdges(Shape& shape) {
    // Simple edge coloring: assign colors 0, 1, 2 (R, G, B) to edges
    // ensuring adjacent edges have different colors

    int currentColor = 0;
    for (auto& contour : shape.contours) {
        for (size_t i = 0; i < contour.edges.size(); ++i) {
            contour.edges[i].color = currentColor;

            // Check angle between this edge and next
            if (i + 1 < contour.edges.size()) {
                Vec2 dir1 = contour.edges[i].p1 - contour.edges[i].p0;
                Vec2 dir2 = contour.edges[i + 1].p1 - contour.edges[i + 1].p0;
                dir1 = dir1.normalized();
                dir2 = dir2.normalized();

                float cross = dir1.x * dir2.y - dir1.y * dir2.x;
                if (std::abs(cross) > 0.5f) {
                    // Sharp corner - change color
                    currentColor = (currentColor + 1) % 3;
                }
            }
        }
    }
}

Vec3 SDFGenerator::msdfDistance(const Shape& shape, Vec2 point) const {
    Vec3 distances(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());

    for (const auto& contour : shape.contours) {
        for (const auto& edge : contour.edges) {
            float dist = distanceToEdge(edge, point);
            int channel = edge.color;

            if (dist < std::abs(distances[channel])) {
                // Determine sign for this edge
                int winding = calculateWinding(contour, point);
                float signedDist = (winding != 0) ? -dist : dist;
                distances[channel] = signedDist;
            }
        }
    }

    return distances;
}

// ============================================================================
// SDF Generation
// ============================================================================

SDFGlyphBitmap SDFGenerator::generate(const GlyphOutline& outline, float fontSize, int16_t unitsPerEm) {
    SDFGlyphBitmap result;

    if (outline.contours.empty()) {
        // Empty glyph (e.g., space)
        result.width = 1;
        result.height = 1;
        result.pixels.resize(1, 128); // Neutral distance
        result.advanceWidth = outline.advanceWidth * fontSize / unitsPerEm;
        return result;
    }

    // Calculate scale and dimensions
    float scale = fontSize / static_cast<float>(unitsPerEm);
    uint32_t padding = static_cast<uint32_t>(std::ceil(spread_));
    result.padding = padding;

    // Glyph bounds in pixels
    float pxMinX = outline.xMin * scale;
    float pxMinY = outline.yMin * scale;
    float pxMaxX = outline.xMax * scale;
    float pxMaxY = outline.yMax * scale;

    uint32_t glyphWidth = static_cast<uint32_t>(std::ceil(pxMaxX - pxMinX)) + 1;
    uint32_t glyphHeight = static_cast<uint32_t>(std::ceil(pxMaxY - pxMinY)) + 1;

    result.width = glyphWidth + padding * 2;
    result.height = glyphHeight + padding * 2;
    result.channels = (mode_ == SDFMode::SDF) ? 1 : (mode_ == SDFMode::MSDF) ? 3 : 4;

    result.bearingX = pxMinX - padding;
    result.bearingY = pxMaxY + padding; // Note: Y is typically from baseline
    result.advanceWidth = outline.advanceWidth * scale;

    // Convert outline to shape
    Shape shape = outlineToShape(outline, scale);

    if (mode_ != SDFMode::SDF) {
        colorEdges(shape);
    }

    // Generate distance field
    result.pixels.resize(result.width * result.height * result.channels);

    for (uint32_t y = 0; y < result.height; ++y) {
        for (uint32_t x = 0; x < result.width; ++x) {
            // Convert pixel coord to glyph space
            Vec2 point;
            point.x = pxMinX - padding + x + 0.5f;
            point.y = pxMaxY + padding - y - 0.5f; // Flip Y

            if (mode_ == SDFMode::SDF) {
                float dist = signedDistance(shape, point);

                // Normalize to 0-255 range
                // Distance of -spread maps to 0, +spread maps to 255
                float normalized = (dist / spread_) * 0.5f + 0.5f;
                normalized = std::clamp(normalized, 0.0f, 1.0f);

                result.pixels[y * result.width + x] = static_cast<uint8_t>(normalized * 255.0f);
            } else {
                // MSDF
                Vec3 dist = msdfDistance(shape, point);

                for (int c = 0; c < 3; ++c) {
                    float normalized = (dist[c] / spread_) * 0.5f + 0.5f;
                    normalized = std::clamp(normalized, 0.0f, 1.0f);
                    result.pixels[(y * result.width + x) * result.channels + c] = static_cast<uint8_t>(normalized * 255.0f);
                }

                if (mode_ == SDFMode::MTSDF) {
                    // Alpha channel = true SDF
                    float trueDist = signedDistance(shape, point);
                    float normalized = (trueDist / spread_) * 0.5f + 0.5f;
                    normalized = std::clamp(normalized, 0.0f, 1.0f);
                    result.pixels[(y * result.width + x) * result.channels + 3] = static_cast<uint8_t>(normalized * 255.0f);
                }
            }
        }
    }

    return result;
}

} // namespace dakt::gui
