#ifndef DAKT_GUI_SHAPE_HPP
#define DAKT_GUI_SHAPE_HPP

#include "WidgetBase.hpp"
#include <cmath>

namespace dakt::gui {

/**
 * @brief Shape type enumeration
 */
enum class ShapeType { Rectangle, RoundedRectangle, Circle, Ellipse, Triangle, Pentagon, Hexagon, Octagon, Star, Diamond, Cross, Arrow, Custom };

/**
 * @brief Base shape widget for geometric shapes
 */
class DAKT_GUI_API Shape : public Widget {
  public:
    Shape();
    explicit Shape(ShapeType type);
    Shape(ShapeType type, const Vec2& size);

    ShapeType getShapeType() const { return shapeType_; }
    void setShapeType(ShapeType type) {
        shapeType_ = type;
        markDirty();
    }

    Color getFillColor() const { return fillColor_; }
    void setFillColor(const Color& color) {
        fillColor_ = color;
        markDirty();
    }

    Color getStrokeColor() const { return strokeColor_; }
    void setStrokeColor(const Color& color) {
        strokeColor_ = color;
        markDirty();
    }

    float getStrokeWidth() const { return strokeWidth_; }
    void setStrokeWidth(float width) {
        strokeWidth_ = width;
        markDirty();
    }

    float getRotation() const { return rotation_; }
    void setRotation(float degrees) {
        rotation_ = degrees;
        markDirty();
    }

    float getBorderRadius() const { return borderRadius_; }
    void setBorderRadius(float radius) {
        borderRadius_ = radius;
        markDirty();
    }

    // For star shape
    int getPointCount() const { return pointCount_; }
    void setPointCount(int count) {
        pointCount_ = count;
        markDirty();
    }

    float getInnerRadius() const { return innerRadius_; }
    void setInnerRadius(float ratio) {
        innerRadius_ = ratio;
        markDirty();
    }

    Vec2 measureContent() override;
    void drawContent(DrawList& drawList) override;

  protected:
    virtual void generateVertices(std::vector<Vec2>& vertices) const;

    ShapeType shapeType_ = ShapeType::Rectangle;
    Color fillColor_{100, 100, 100, 255};
    Color strokeColor_{150, 150, 150, 255};
    float strokeWidth_ = 0.0f;
    float rotation_ = 0.0f;
    float borderRadius_ = 0.0f;
    int pointCount_ = 5;       // For star/polygon shapes
    float innerRadius_ = 0.5f; // For star shape (inner radius ratio)
};

/**
 * @brief Hexagon shape widget
 */
class DAKT_GUI_API Hexagon : public Shape {
  public:
    Hexagon();
    explicit Hexagon(float size);
    Hexagon(float size, const Color& color);

    bool isPointyTop() const { return pointyTop_; }
    void setPointyTop(bool pointy) {
        pointyTop_ = pointy;
        markDirty();
    }

  protected:
    void generateVertices(std::vector<Vec2>& vertices) const override;

  private:
    bool pointyTop_ = true;
};

/**
 * @brief Circle shape widget
 */
class DAKT_GUI_API Circle : public Shape {
  public:
    Circle();
    explicit Circle(float radius);
    Circle(float radius, const Color& color);

    float getRadius() const { return radius_; }
    void setRadius(float radius);

    int getSegments() const { return segments_; }
    void setSegments(int segments) {
        segments_ = segments;
        markDirty();
    }

    Vec2 measureContent() override;

  protected:
    void generateVertices(std::vector<Vec2>& vertices) const override;

  private:
    float radius_ = 50.0f;
    int segments_ = 32;
};

/**
 * @brief Triangle shape widget
 */
class DAKT_GUI_API Triangle : public Shape {
  public:
    enum class TriangleType { Equilateral, Isosceles, Right, Custom };

    Triangle();
    explicit Triangle(float size);
    Triangle(float size, TriangleType type);

    TriangleType getTriangleType() const { return triangleType_; }
    void setTriangleType(TriangleType type) {
        triangleType_ = type;
        markDirty();
    }

    // For custom triangles
    void setCustomPoints(const Vec2& p1, const Vec2& p2, const Vec2& p3);

  protected:
    void generateVertices(std::vector<Vec2>& vertices) const override;

  private:
    TriangleType triangleType_ = TriangleType::Equilateral;
    Vec2 customPoints_[3];
};

/**
 * @brief Star shape widget
 */
class DAKT_GUI_API Star : public Shape {
  public:
    Star();
    Star(int points, float outerRadius, float innerRadius);

    void setPoints(int points) { setPointCount(points); }
    int getPoints() const { return getPointCount(); }

    float getOuterRadius() const { return outerRadius_; }
    void setOuterRadius(float radius) {
        outerRadius_ = radius;
        markDirty();
    }

    Vec2 measureContent() override;

  protected:
    void generateVertices(std::vector<Vec2>& vertices) const override;

  private:
    float outerRadius_ = 50.0f;
};

/**
 * @brief Pentagon shape widget
 */
class DAKT_GUI_API Pentagon : public Shape {
  public:
    Pentagon();
    explicit Pentagon(float size);
    Pentagon(float size, const Color& color);

  protected:
    void generateVertices(std::vector<Vec2>& vertices) const override;
};

/**
 * @brief Octagon shape widget
 */
class DAKT_GUI_API Octagon : public Shape {
  public:
    Octagon();
    explicit Octagon(float size);
    Octagon(float size, const Color& color);

  protected:
    void generateVertices(std::vector<Vec2>& vertices) const override;
};

/**
 * @brief Diamond/Rhombus shape widget
 */
class DAKT_GUI_API Diamond : public Shape {
  public:
    Diamond();
    explicit Diamond(float size);
    Diamond(float width, float height);

  protected:
    void generateVertices(std::vector<Vec2>& vertices) const override;
};

/**
 * @brief Arrow shape widget
 */
class DAKT_GUI_API Arrow : public Shape {
  public:
    enum class ArrowDirection { Up, Down, Left, Right };

    Arrow();
    Arrow(ArrowDirection direction, float size);

    ArrowDirection getDirection() const { return direction_; }
    void setDirection(ArrowDirection dir) {
        direction_ = dir;
        markDirty();
    }

    float getHeadRatio() const { return headRatio_; }
    void setHeadRatio(float ratio) {
        headRatio_ = ratio;
        markDirty();
    }

    float getShaftRatio() const { return shaftRatio_; }
    void setShaftRatio(float ratio) {
        shaftRatio_ = ratio;
        markDirty();
    }

  protected:
    void generateVertices(std::vector<Vec2>& vertices) const override;

  private:
    ArrowDirection direction_ = ArrowDirection::Right;
    float headRatio_ = 0.4f;
    float shaftRatio_ = 0.4f;
};

/**
 * @brief Cross/Plus shape widget
 */
class DAKT_GUI_API Cross : public Shape {
  public:
    Cross();
    explicit Cross(float size);
    Cross(float size, float thickness);

    float getThickness() const { return thickness_; }
    void setThickness(float thickness) {
        thickness_ = thickness;
        markDirty();
    }

  protected:
    void generateVertices(std::vector<Vec2>& vertices) const override;

  private:
    float thickness_ = 0.3f; // Relative to size
};

} // namespace dakt::gui

#endif // DAKT_GUI_SHAPE_HPP
