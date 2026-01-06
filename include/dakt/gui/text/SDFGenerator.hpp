#ifndef DAKTLIB_GUI_SDF_GENERATOR_HPP
#define DAKTLIB_GUI_SDF_GENERATOR_HPP

#include "../core/Types.hpp"
#include "TTFParser.hpp"
#include <cstdint>
#include <vector>

namespace dakt::gui {

/**
 * @brief SDF (Signed Distance Field) generation modes
 */
enum class SDFMode {
    SDF,  // Single-channel SDF
    MSDF, // Multi-channel SDF (sharper edges)
    MTSDF // MSDF + true distance in alpha channel
};

/**
 * @brief Result of SDF glyph rasterization
 */
struct SDFGlyphBitmap {
    std::vector<uint8_t> pixels;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t channels = 1; // 1 for SDF, 3 for MSDF, 4 for MTSDF

    // Glyph metrics in pixels (at target size)
    float bearingX = 0;
    float bearingY = 0;
    float advanceWidth = 0;

    // Padding around glyph for SDF spread
    uint32_t padding = 0;
};

/**
 * @brief Bezier curve segment for glyph outline
 */
struct EdgeSegment {
    enum Type { Linear, Quadratic, Cubic };
    Type type;
    Vec2 p0, p1, p2, p3; // Control points (p2, p3 only for quad/cubic)
    int color = 0;       // For MSDF edge coloring
};

/**
 * @brief Closed contour of edge segments
 */
struct Contour {
    std::vector<EdgeSegment> edges;
    int winding = 0; // +1 clockwise, -1 counter-clockwise
};

/**
 * @brief Shape composed of contours for SDF generation
 */
struct Shape {
    std::vector<Contour> contours;
    float inverseYAxis = true; // Font Y is typically inverted
};

/**
 * @brief SDF/MSDF generator from glyph outlines
 */
class SDFGenerator {
  public:
    SDFGenerator();
    ~SDFGenerator();

    /**
     * Set the pixel range for distance field spread
     * Higher values = more spread/blur, better for scaling
     */
    void setSpread(float spread) { spread_ = spread; }
    float getSpread() const { return spread_; }

    /**
     * Set SDF generation mode
     */
    void setMode(SDFMode mode) { mode_ = mode; }
    SDFMode getMode() const { return mode_; }

    /**
     * Generate SDF bitmap from glyph outline
     * @param outline Glyph outline from TTFParser
     * @param fontSize Target font size in pixels
     * @param unitsPerEm Font's units per em
     * @return SDFGlyphBitmap with pixel data
     */
    SDFGlyphBitmap generate(const GlyphOutline& outline, float fontSize, int16_t unitsPerEm);

    /**
     * Convert glyph outline to shape for SDF processing
     */
    Shape outlineToShape(const GlyphOutline& outline, float scale);

    /**
     * Calculate signed distance from point to shape
     */
    float signedDistance(const Shape& shape, Vec2 point) const;

    /**
     * Calculate signed distance to single edge segment
     */
    float distanceToEdge(const EdgeSegment& edge, Vec2 point) const;

  private:
    // Edge coloring for MSDF (assigns colors to edges to prevent artifacts)
    void colorEdges(Shape& shape);

    // Distance calculations
    float distanceToLine(Vec2 p, Vec2 a, Vec2 b) const;
    float distanceToQuadratic(Vec2 p, Vec2 p0, Vec2 p1, Vec2 p2) const;
    float distanceToCubic(Vec2 p, Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3) const;

    // Winding number calculation for inside/outside test
    int calculateWinding(const Contour& contour, Vec2 point) const;

    // MSDF-specific: calculate per-channel distances
    Vec3 msdfDistance(const Shape& shape, Vec2 point) const;

    float spread_ = 4.0f;
    SDFMode mode_ = SDFMode::SDF;
};

} // namespace dakt::gui

#endif
