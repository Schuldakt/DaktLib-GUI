#include "dakt/gui/retained/widgets/Shape.hpp"
#include "dakt/gui/draw/DrawList.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace dakt::gui {

// ============================================================================
// Shape Base Class
// ============================================================================

Shape::Shape() : Widget() { shapeType_ = ShapeType::Rectangle; }

Shape::Shape(ShapeType type) : Widget(), shapeType_(type) {}

Shape::Shape(ShapeType type, const Vec2& size) : Widget(), shapeType_(type) { setPreferredSize(size); }

Vec2 Shape::measureContent() {
    if (preferredSize_.x > 0 && preferredSize_.y > 0) {
        return preferredSize_;
    }
    return Vec2(50, 50); // Default size
}

void Shape::generateVertices(std::vector<Vec2>& vertices) const {
    float w = bounds_.width;
    float h = bounds_.height;
    float cx = bounds_.x + w / 2;
    float cy = bounds_.y + h / 2;

    switch (shapeType_) {
    case ShapeType::Rectangle:
    case ShapeType::RoundedRectangle:
        vertices = {{bounds_.x, bounds_.y}, {bounds_.x + w, bounds_.y}, {bounds_.x + w, bounds_.y + h}, {bounds_.x, bounds_.y + h}};
        break;

    case ShapeType::Diamond:
        vertices = {{cx, bounds_.y}, {bounds_.x + w, cy}, {cx, bounds_.y + h}, {bounds_.x, cy}};
        break;

    default:
        // For polygon shapes, generate based on point count
        int points = pointCount_;
        float radius = std::min(w, h) / 2;
        for (int i = 0; i < points; ++i) {
            float angle = (2 * M_PI * i / points) - M_PI / 2 + (rotation_ * M_PI / 180);
            vertices.push_back({cx + radius * std::cos(angle), cy + radius * std::sin(angle)});
        }
        break;
    }
}

void Shape::drawContent(DrawList& drawList) {
    std::vector<Vec2> vertices;
    generateVertices(vertices);

    if (vertices.size() < 3)
        return;

    // Draw filled polygon using triangle fan
    if (fillColor_.a > 0) {
        // Use triangle fan from center
        Vec2 center(bounds_.x + bounds_.width / 2, bounds_.y + bounds_.height / 2);

        for (size_t i = 0; i < vertices.size(); ++i) {
            size_t next = (i + 1) % vertices.size();
            drawList.drawTriangleFilled(center, vertices[i], vertices[next], fillColor_);
        }
    }

    // Draw stroke as lines
    if (strokeWidth_ > 0 && strokeColor_.a > 0) {
        for (size_t i = 0; i < vertices.size(); ++i) {
            size_t next = (i + 1) % vertices.size();
            drawList.drawLine(vertices[i], vertices[next], strokeColor_, strokeWidth_);
        }
    }
}

// ============================================================================
// Hexagon
// ============================================================================

Hexagon::Hexagon() : Shape(ShapeType::Hexagon) { pointCount_ = 6; }

Hexagon::Hexagon(float size) : Shape(ShapeType::Hexagon) {
    pointCount_ = 6;
    setPreferredSize(Vec2(size, size));
}

Hexagon::Hexagon(float size, const Color& color) : Shape(ShapeType::Hexagon) {
    pointCount_ = 6;
    setPreferredSize(Vec2(size, size));
    setFillColor(color);
}

void Hexagon::generateVertices(std::vector<Vec2>& vertices) const {
    float w = bounds_.width;
    float h = bounds_.height;
    float cx = bounds_.x + w / 2;
    float cy = bounds_.y + h / 2;
    float radius = std::min(w, h) / 2;

    // Pointy-top vs flat-top hexagon
    float startAngle = pointyTop_ ? -M_PI / 2 : 0;

    for (int i = 0; i < 6; ++i) {
        float angle = startAngle + (2 * M_PI * i / 6) + (rotation_ * M_PI / 180);
        vertices.push_back({cx + radius * std::cos(angle), cy + radius * std::sin(angle)});
    }
}

// ============================================================================
// Circle
// ============================================================================

Circle::Circle() : Shape(ShapeType::Circle) { radius_ = 50.0f; }

Circle::Circle(float radius) : Shape(ShapeType::Circle), radius_(radius) { setPreferredSize(Vec2(radius * 2, radius * 2)); }

Circle::Circle(float radius, const Color& color) : Shape(ShapeType::Circle), radius_(radius) {
    setPreferredSize(Vec2(radius * 2, radius * 2));
    setFillColor(color);
}

void Circle::setRadius(float radius) {
    radius_ = radius;
    setPreferredSize(Vec2(radius * 2, radius * 2));
    markDirty();
}

Vec2 Circle::measureContent() { return Vec2(radius_ * 2, radius_ * 2); }

void Circle::generateVertices(std::vector<Vec2>& vertices) const {
    float cx = bounds_.x + bounds_.width / 2;
    float cy = bounds_.y + bounds_.height / 2;
    float r = std::min(bounds_.width, bounds_.height) / 2;

    for (int i = 0; i < segments_; ++i) {
        float angle = 2 * M_PI * i / segments_;
        vertices.push_back({cx + r * std::cos(angle), cy + r * std::sin(angle)});
    }
}

// ============================================================================
// Triangle
// ============================================================================

Triangle::Triangle() : Shape(ShapeType::Triangle) { pointCount_ = 3; }

Triangle::Triangle(float size) : Shape(ShapeType::Triangle) {
    pointCount_ = 3;
    setPreferredSize(Vec2(size, size));
}

Triangle::Triangle(float size, TriangleType type) : Shape(ShapeType::Triangle), triangleType_(type) {
    pointCount_ = 3;
    setPreferredSize(Vec2(size, size));
}

void Triangle::setCustomPoints(const Vec2& p1, const Vec2& p2, const Vec2& p3) {
    triangleType_ = TriangleType::Custom;
    customPoints_[0] = p1;
    customPoints_[1] = p2;
    customPoints_[2] = p3;
    markDirty();
}

void Triangle::generateVertices(std::vector<Vec2>& vertices) const {
    float w = bounds_.width;
    float h = bounds_.height;

    if (triangleType_ == TriangleType::Custom) {
        for (int i = 0; i < 3; ++i) {
            vertices.push_back({bounds_.x + customPoints_[i].x * w, bounds_.y + customPoints_[i].y * h});
        }
        return;
    }

    float cx = bounds_.x + w / 2;
    float cy = bounds_.y + h / 2;

    switch (triangleType_) {
    case TriangleType::Equilateral: {
        float r = std::min(w, h) / 2;
        for (int i = 0; i < 3; ++i) {
            float angle = -M_PI / 2 + (2 * M_PI * i / 3) + (rotation_ * M_PI / 180);
            vertices.push_back({cx + r * std::cos(angle), cy + r * std::sin(angle)});
        }
        break;
    }
    case TriangleType::Isosceles:
        vertices = {{cx, bounds_.y}, {bounds_.x + w, bounds_.y + h}, {bounds_.x, bounds_.y + h}};
        break;
    case TriangleType::Right:
        vertices = {{bounds_.x, bounds_.y}, {bounds_.x + w, bounds_.y + h}, {bounds_.x, bounds_.y + h}};
        break;
    default:
        vertices = {{cx, bounds_.y}, {bounds_.x + w, bounds_.y + h}, {bounds_.x, bounds_.y + h}};
        break;
    }
}

// ============================================================================
// Star
// ============================================================================

Star::Star() : Shape(ShapeType::Star) {
    pointCount_ = 5;
    innerRadius_ = 0.4f;
}

Star::Star(int points, float outerRadius, float innerRatio) : Shape(ShapeType::Star) {
    pointCount_ = points;
    outerRadius_ = outerRadius;
    innerRadius_ = innerRatio;
    setPreferredSize(Vec2(outerRadius * 2, outerRadius * 2));
}

Vec2 Star::measureContent() { return Vec2(outerRadius_ * 2, outerRadius_ * 2); }

void Star::generateVertices(std::vector<Vec2>& vertices) const {
    float cx = bounds_.x + bounds_.width / 2;
    float cy = bounds_.y + bounds_.height / 2;
    float outerR = std::min(bounds_.width, bounds_.height) / 2;
    float innerR = outerR * innerRadius_;
    int points = pointCount_;

    for (int i = 0; i < points * 2; ++i) {
        float angle = (M_PI * i / points) - M_PI / 2 + (rotation_ * M_PI / 180);
        float r = (i % 2 == 0) ? outerR : innerR;
        vertices.push_back({cx + r * std::cos(angle), cy + r * std::sin(angle)});
    }
}

// ============================================================================
// Pentagon
// ============================================================================

Pentagon::Pentagon() : Shape(ShapeType::Pentagon) { pointCount_ = 5; }

Pentagon::Pentagon(float size) : Shape(ShapeType::Pentagon) {
    pointCount_ = 5;
    setPreferredSize(Vec2(size, size));
}

Pentagon::Pentagon(float size, const Color& color) : Shape(ShapeType::Pentagon) {
    pointCount_ = 5;
    setPreferredSize(Vec2(size, size));
    setFillColor(color);
}

void Pentagon::generateVertices(std::vector<Vec2>& vertices) const {
    float cx = bounds_.x + bounds_.width / 2;
    float cy = bounds_.y + bounds_.height / 2;
    float radius = std::min(bounds_.width, bounds_.height) / 2;

    for (int i = 0; i < 5; ++i) {
        float angle = -M_PI / 2 + (2 * M_PI * i / 5) + (rotation_ * M_PI / 180);
        vertices.push_back({cx + radius * std::cos(angle), cy + radius * std::sin(angle)});
    }
}

// ============================================================================
// Octagon
// ============================================================================

Octagon::Octagon() : Shape(ShapeType::Octagon) { pointCount_ = 8; }

Octagon::Octagon(float size) : Shape(ShapeType::Octagon) {
    pointCount_ = 8;
    setPreferredSize(Vec2(size, size));
}

Octagon::Octagon(float size, const Color& color) : Shape(ShapeType::Octagon) {
    pointCount_ = 8;
    setPreferredSize(Vec2(size, size));
    setFillColor(color);
}

void Octagon::generateVertices(std::vector<Vec2>& vertices) const {
    float cx = bounds_.x + bounds_.width / 2;
    float cy = bounds_.y + bounds_.height / 2;
    float radius = std::min(bounds_.width, bounds_.height) / 2;

    for (int i = 0; i < 8; ++i) {
        float angle = (M_PI / 8) + (2 * M_PI * i / 8) + (rotation_ * M_PI / 180);
        vertices.push_back({cx + radius * std::cos(angle), cy + radius * std::sin(angle)});
    }
}

// ============================================================================
// Diamond
// ============================================================================

Diamond::Diamond() : Shape(ShapeType::Diamond) {}

Diamond::Diamond(float size) : Shape(ShapeType::Diamond) { setPreferredSize(Vec2(size, size)); }

Diamond::Diamond(float width, float height) : Shape(ShapeType::Diamond) { setPreferredSize(Vec2(width, height)); }

void Diamond::generateVertices(std::vector<Vec2>& vertices) const {
    float cx = bounds_.x + bounds_.width / 2;
    float cy = bounds_.y + bounds_.height / 2;

    vertices = {{cx, bounds_.y}, {bounds_.x + bounds_.width, cy}, {cx, bounds_.y + bounds_.height}, {bounds_.x, cy}};
}

// ============================================================================
// Arrow
// ============================================================================

Arrow::Arrow() : Shape(ShapeType::Arrow) {}

Arrow::Arrow(ArrowDirection direction, float size) : Shape(ShapeType::Arrow), direction_(direction) { setPreferredSize(Vec2(size, size)); }

void Arrow::generateVertices(std::vector<Vec2>& vertices) const {
    float w = bounds_.width;
    float h = bounds_.height;
    float x = bounds_.x;
    float y = bounds_.y;

    float headW = w * headRatio_;
    float shaftW = w * shaftRatio_;
    float shaftOffset = (h - h * shaftRatio_) / 2;

    switch (direction_) {
    case ArrowDirection::Right:
        vertices = {{x, y + shaftOffset}, {x + w - headW, y + shaftOffset}, {x + w - headW, y}, {x + w, y + h / 2}, {x + w - headW, y + h}, {x + w - headW, y + h - shaftOffset}, {x, y + h - shaftOffset}};
        break;
    case ArrowDirection::Left:
        vertices = {{x + w, y + shaftOffset}, {x + headW, y + shaftOffset}, {x + headW, y}, {x, y + h / 2}, {x + headW, y + h}, {x + headW, y + h - shaftOffset}, {x + w, y + h - shaftOffset}};
        break;
    case ArrowDirection::Down:
        vertices = {{x + shaftOffset, y}, {x + w - shaftOffset, y}, {x + w - shaftOffset, y + h - headW}, {x + w, y + h - headW}, {x + w / 2, y + h}, {x, y + h - headW}, {x + shaftOffset, y + h - headW}};
        break;
    case ArrowDirection::Up:
        vertices = {{x + w / 2, y}, {x + w, y + headW}, {x + w - shaftOffset, y + headW}, {x + w - shaftOffset, y + h}, {x + shaftOffset, y + h}, {x + shaftOffset, y + headW}, {x, y + headW}};
        break;
    }
}

// ============================================================================
// Cross
// ============================================================================

Cross::Cross() : Shape(ShapeType::Cross) {}

Cross::Cross(float size) : Shape(ShapeType::Cross) { setPreferredSize(Vec2(size, size)); }

Cross::Cross(float size, float thickness) : Shape(ShapeType::Cross), thickness_(thickness) { setPreferredSize(Vec2(size, size)); }

void Cross::generateVertices(std::vector<Vec2>& vertices) const {
    float w = bounds_.width;
    float h = bounds_.height;
    float x = bounds_.x;
    float y = bounds_.y;
    float t = std::min(w, h) * thickness_;
    float cx = x + w / 2;
    float cy = y + h / 2;

    // Cross shape (12 vertices)
    vertices = {{cx - t / 2, y},     {cx + t / 2, y},     {cx + t / 2, cy - t / 2}, {x + w, cy - t / 2}, {x + w, cy + t / 2}, {cx + t / 2, cy + t / 2},
                {cx + t / 2, y + h}, {cx - t / 2, y + h}, {cx - t / 2, cy + t / 2}, {x, cy + t / 2},     {x, cy - t / 2},     {cx - t / 2, cy - t / 2}};
}

} // namespace dakt::gui
