/**
 * @file showcase.cpp
 * @brief Interactive showcase demonstrating DaktLib-GUI widgets
 *
 * This example creates a window with various widgets to visually verify
 * the GUI library functionality.
 *
 * Requires GLFW3 for windowing (optional dependency for examples only).
 * Uses OpenGL for displaying the software-rendered framebuffer.
 */

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <GLFW/glfw3.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "dakt/gui/GUI.hpp"
#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/draw/DrawList.hpp"
#include "dakt/gui/immediate/Immediate.hpp"
#include "dakt/gui/input/Input.hpp"
#include "dakt/gui/retained/Containers.hpp"
#include "dakt/gui/retained/Widgets.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace dakt::gui;

// ============================================================================
// Application State
// ============================================================================

struct AppState {
    // Immediate mode widget state
    bool checkboxValue = false;
    float sliderValue = 0.5f;
    int selectedTab = 0;
    char textBuffer[256] = "Hello, DaktLib!";

    // Shape demo state
    float shapeRotation = 0.0f;
    float progressValue = 0.0f;
    bool progressForward = true;

    // Retained mode container
    std::unique_ptr<UIContainer> retainedUI;

    // Demo toggles
    bool showImmediateDemo = true;
    bool showRetainedDemo = true;
    bool showShapesDemo = true;
    bool showStyleDemo = false;

    // Window state
    int windowWidth = 1280;
    int windowHeight = 720;
};

static AppState g_app;

// ============================================================================
// Software Renderer (for demonstration without GPU backend)
// ============================================================================

/**
 * Simple software renderer that draws to an RGBA pixel buffer.
 * This allows us to visualize the DrawList output without a GPU.
 */
class SoftwareRenderer {
  public:
    SoftwareRenderer(int width, int height) : width_(width), height_(height) { pixels_.resize(width * height * 4, 0); }

    void resize(int width, int height) {
        width_ = width;
        height_ = height;
        pixels_.resize(width * height * 4, 0);
    }

    void clear(Color color) {
        for (size_t i = 0; i < pixels_.size(); i += 4) {
            pixels_[i + 0] = color.r;
            pixels_[i + 1] = color.g;
            pixels_[i + 2] = color.b;
            pixels_[i + 3] = color.a;
        }
    }

    void drawRect(const Rect& rect, Color color) {
        int x0 = static_cast<int>(std::max(0.0f, rect.x));
        int y0 = static_cast<int>(std::max(0.0f, rect.y));
        int x1 = static_cast<int>(std::min(static_cast<float>(width_), rect.x + rect.width));
        int y1 = static_cast<int>(std::min(static_cast<float>(height_), rect.y + rect.height));

        for (int y = y0; y < y1; ++y) {
            for (int x = x0; x < x1; ++x) {
                setPixel(x, y, color);
            }
        }
    }

    void drawRectOutline(const Rect& rect, Color color, float thickness = 1.0f) {
        int t = static_cast<int>(thickness);
        // Top
        drawRect(Rect(rect.x, rect.y, rect.width, t), color);
        // Bottom
        drawRect(Rect(rect.x, rect.y + rect.height - t, rect.width, t), color);
        // Left
        drawRect(Rect(rect.x, rect.y, t, rect.height), color);
        // Right
        drawRect(Rect(rect.x + rect.width - t, rect.y, t, rect.height), color);
    }

    void drawText(const Vec2& pos, const char* text, Color color, float fontSize = 14.0f) {
        // Simple placeholder text rendering (character blocks)
        float x = pos.x;
        float charWidth = fontSize * 0.6f;
        float charHeight = fontSize;

        while (*text) {
            if (*text != ' ') {
                drawRect(Rect(x + 1, pos.y + 1, charWidth - 2, charHeight - 2), color);
            }
            x += charWidth;
            ++text;
        }
    }

    void render(const DrawList& drawList) {
        // Process draw commands
        for (const auto& cmd : drawList.getCommands()) {
            // Each command references vertices/indices
            // For software rendering, we interpret the vertex data
        }

        // For now, process the internal shape commands
        // This is a simplified renderer for demonstration
    }

    const uint8_t* getPixels() const { return pixels_.data(); }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

  private:
    void setPixel(int x, int y, Color color) {
        if (x < 0 || x >= width_ || y < 0 || y >= height_)
            return;

        size_t idx = (y * width_ + x) * 4;

        // Alpha blending
        float srcA = color.a / 255.0f;
        float invA = 1.0f - srcA;

        pixels_[idx + 0] = static_cast<uint8_t>(color.r * srcA + pixels_[idx + 0] * invA);
        pixels_[idx + 1] = static_cast<uint8_t>(color.g * srcA + pixels_[idx + 1] * invA);
        pixels_[idx + 2] = static_cast<uint8_t>(color.b * srcA + pixels_[idx + 2] * invA);
        pixels_[idx + 3] = 255;
    }

    int width_;
    int height_;
    std::vector<uint8_t> pixels_;
};

static std::unique_ptr<SoftwareRenderer> g_renderer;

// ============================================================================
// GLFW Callbacks
// ============================================================================

static void glfwErrorCallback(int error, const char* description) { fprintf(stderr, "GLFW Error %d: %s\n", error, description); }

static void glfwFramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    g_app.windowWidth = width;
    g_app.windowHeight = height;
    if (g_renderer) {
        g_renderer->resize(width, height);
    }
}

static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    // Tab to switch demos
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        if (mods & GLFW_MOD_CONTROL) {
            g_app.showShapesDemo = !g_app.showShapesDemo;
        } else if (mods & GLFW_MOD_SHIFT) {
            g_app.showRetainedDemo = !g_app.showRetainedDemo;
        } else {
            g_app.showImmediateDemo = !g_app.showImmediateDemo;
        }
    }

    // Number keys for tabs
    if (action == GLFW_PRESS && key >= GLFW_KEY_1 && key <= GLFW_KEY_3) {
        g_app.selectedTab = key - GLFW_KEY_1;
    }
}

// ============================================================================
// Immediate Mode Demo
// ============================================================================

void renderImmediateModeDemo(SoftwareRenderer& renderer, float x, float y, float width, float height) {
    // Panel background
    renderer.drawRect(Rect(x, y, width, height), Color(30, 30, 35, 240));
    renderer.drawRectOutline(Rect(x, y, width, height), Color(60, 60, 70, 255), 2);

    // Title
    renderer.drawText(Vec2(x + 10, y + 10), "Immediate Mode Widgets", Color(255, 255, 255, 255), 16);

    float contentY = y + 40;
    float padding = 10;

    // Button
    Rect buttonRect(x + padding, contentY, 120, 30);
    renderer.drawRect(buttonRect, Color(60, 120, 200, 255));
    renderer.drawText(Vec2(buttonRect.x + 10, buttonRect.y + 8), "Click Me", Color(255, 255, 255, 255));
    contentY += 40;

    // Checkbox
    Rect checkboxRect(x + padding, contentY, 18, 18);
    renderer.drawRect(checkboxRect, Color(50, 50, 60, 255));
    renderer.drawRectOutline(checkboxRect, Color(100, 100, 120, 255));
    if (g_app.checkboxValue) {
        renderer.drawRect(Rect(checkboxRect.x + 4, checkboxRect.y + 4, 10, 10), Color(60, 180, 60, 255));
    }
    renderer.drawText(Vec2(checkboxRect.x + 25, contentY), "Enable Feature", Color(200, 200, 200, 255));
    contentY += 30;

    // Slider
    renderer.drawText(Vec2(x + padding, contentY), "Volume:", Color(200, 200, 200, 255));
    contentY += 20;

    Rect trackRect(x + padding, contentY, width - 2 * padding, 8);
    renderer.drawRect(trackRect, Color(40, 40, 50, 255));

    float fillWidth = trackRect.width * g_app.sliderValue;
    renderer.drawRect(Rect(trackRect.x, trackRect.y, fillWidth, trackRect.height), Color(60, 140, 220, 255));

    float thumbX = trackRect.x + fillWidth - 6;
    renderer.drawRect(Rect(thumbX, trackRect.y - 4, 12, 16), Color(220, 220, 230, 255));
    contentY += 30;

    // Text Input
    renderer.drawText(Vec2(x + padding, contentY), "Name:", Color(200, 200, 200, 255));
    contentY += 20;

    Rect inputRect(x + padding, contentY, width - 2 * padding, 28);
    renderer.drawRect(inputRect, Color(35, 35, 45, 255));
    renderer.drawRectOutline(inputRect, Color(80, 80, 100, 255));
    renderer.drawText(Vec2(inputRect.x + 5, inputRect.y + 6), g_app.textBuffer, Color(220, 220, 220, 255));
    contentY += 40;

    // Tabs
    const char* tabs[] = {"General", "Settings", "About"};
    float tabWidth = (width - 2 * padding) / 3;
    for (int i = 0; i < 3; ++i) {
        Rect tabRect(x + padding + i * tabWidth, contentY, tabWidth - 2, 28);
        Color tabColor = (i == g_app.selectedTab) ? Color(60, 60, 80, 255) : Color(40, 40, 50, 255);
        renderer.drawRect(tabRect, tabColor);
        renderer.drawText(Vec2(tabRect.x + 10, tabRect.y + 6), tabs[i], Color(200, 200, 200, 255));
    }
    contentY += 35;

    // Tab content
    Rect contentRect(x + padding, contentY, width - 2 * padding, 100);
    renderer.drawRect(contentRect, Color(40, 40, 50, 255));

    const char* tabContent[] = {"General settings go here.", "Advanced configuration.", "DaktLib-GUI v0.1.0"};
    renderer.drawText(Vec2(contentRect.x + 10, contentRect.y + 10), tabContent[g_app.selectedTab], Color(180, 180, 180, 255));
}

// ============================================================================
// Shapes Demo
// ============================================================================

void drawHexagon(SoftwareRenderer& renderer, float cx, float cy, float radius, Color color, bool pointyTop = true) {
    float startAngle = pointyTop ? -M_PI / 2 : 0;

    // Draw using triangles from center
    for (int i = 0; i < 6; ++i) {
        float angle1 = startAngle + (2 * M_PI * i / 6);
        float angle2 = startAngle + (2 * M_PI * (i + 1) / 6);

        float x1 = cx + radius * std::cos(angle1);
        float y1 = cy + radius * std::sin(angle1);
        float x2 = cx + radius * std::cos(angle2);
        float y2 = cy + radius * std::sin(angle2);

        // Fill triangle by drawing horizontal lines
        for (float t = 0; t <= 1; t += 0.02f) {
            float px1 = cx + t * (x1 - cx);
            float py1 = cy + t * (y1 - cy);
            float px2 = cx + t * (x2 - cx);
            float py2 = cy + t * (y2 - cy);

            int steps = static_cast<int>(std::abs(px2 - px1) + std::abs(py2 - py1)) + 1;
            for (int s = 0; s <= steps; ++s) {
                float u = static_cast<float>(s) / steps;
                int x = static_cast<int>(px1 + u * (px2 - px1));
                int y = static_cast<int>(py1 + u * (py2 - py1));
                renderer.drawRect(Rect(x, y, 2, 2), color);
            }
        }
    }
}

void drawPentagon(SoftwareRenderer& renderer, float cx, float cy, float radius, Color color) {
    for (int i = 0; i < 5; ++i) {
        float angle1 = -M_PI / 2 + (2 * M_PI * i / 5);
        float angle2 = -M_PI / 2 + (2 * M_PI * (i + 1) / 5);

        for (float t = 0; t <= 1; t += 0.02f) {
            float x1 = cx + t * radius * std::cos(angle1);
            float y1 = cy + t * radius * std::sin(angle1);
            float x2 = cx + t * radius * std::cos(angle2);
            float y2 = cy + t * radius * std::sin(angle2);

            int steps = static_cast<int>(std::abs(x2 - x1) + std::abs(y2 - y1)) + 1;
            for (int s = 0; s <= steps; ++s) {
                float u = static_cast<float>(s) / steps;
                int x = static_cast<int>(x1 + u * (x2 - x1));
                int y = static_cast<int>(y1 + u * (y2 - y1));
                renderer.drawRect(Rect(x, y, 2, 2), color);
            }
        }
    }
}

void drawStar(SoftwareRenderer& renderer, float cx, float cy, float outerRadius, float innerRadius, int points, Color color) {
    for (int i = 0; i < points * 2; ++i) {
        float angle1 = (M_PI * i / points) - M_PI / 2;
        float angle2 = (M_PI * (i + 1) / points) - M_PI / 2;
        float r1 = (i % 2 == 0) ? outerRadius : innerRadius;
        float r2 = ((i + 1) % 2 == 0) ? outerRadius : innerRadius;

        float x1 = cx + r1 * std::cos(angle1);
        float y1 = cy + r1 * std::sin(angle1);
        float x2 = cx + r2 * std::cos(angle2);
        float y2 = cy + r2 * std::sin(angle2);

        // Draw triangle from center
        for (float t = 0; t <= 1; t += 0.02f) {
            float px1 = cx + t * (x1 - cx);
            float py1 = cy + t * (y1 - cy);
            float px2 = cx + t * (x2 - cx);
            float py2 = cy + t * (y2 - cy);

            int steps = static_cast<int>(std::abs(px2 - px1) + std::abs(py2 - py1)) + 1;
            for (int s = 0; s <= steps; ++s) {
                float u = static_cast<float>(s) / steps;
                int x = static_cast<int>(px1 + u * (px2 - px1));
                int y = static_cast<int>(py1 + u * (py2 - py1));
                renderer.drawRect(Rect(x, y, 2, 2), color);
            }
        }
    }
}

void drawOctagon(SoftwareRenderer& renderer, float cx, float cy, float radius, Color color) {
    for (int i = 0; i < 8; ++i) {
        float angle1 = (M_PI / 8) + (2 * M_PI * i / 8);
        float angle2 = (M_PI / 8) + (2 * M_PI * (i + 1) / 8);

        for (float t = 0; t <= 1; t += 0.02f) {
            float x1 = cx + t * radius * std::cos(angle1);
            float y1 = cy + t * radius * std::sin(angle1);
            float x2 = cx + t * radius * std::cos(angle2);
            float y2 = cy + t * radius * std::sin(angle2);

            int steps = static_cast<int>(std::abs(x2 - x1) + std::abs(y2 - y1)) + 1;
            for (int s = 0; s <= steps; ++s) {
                float u = static_cast<float>(s) / steps;
                int x = static_cast<int>(x1 + u * (x2 - x1));
                int y = static_cast<int>(y1 + u * (y2 - y1));
                renderer.drawRect(Rect(x, y, 2, 2), color);
            }
        }
    }
}

void renderShapesDemo(SoftwareRenderer& renderer, float x, float y, float width, float height) {
    // Panel background
    renderer.drawRect(Rect(x, y, width, height), Color(35, 30, 40, 240));
    renderer.drawRectOutline(Rect(x, y, width, height), Color(70, 60, 80, 255), 2);

    // Title
    renderer.drawText(Vec2(x + 10, y + 10), "Shapes Gallery", Color(255, 255, 255, 255), 16);

    float shapeSize = 35;
    float spacing = 90;
    float startX = x + 45;
    float startY = y + 60;

    // Row 1: Basic polygons
    renderer.drawText(Vec2(x + 10, startY - 20), "Polygons:", Color(180, 180, 180, 255), 12);

    // Hexagon (pointy top)
    drawHexagon(renderer, startX, startY + shapeSize, shapeSize, Color(255, 100, 100, 255), true);
    renderer.drawText(Vec2(startX - 25, startY + shapeSize * 2 + 10), "Hexagon", Color(150, 150, 150, 255), 10);

    // Pentagon
    drawPentagon(renderer, startX + spacing, startY + shapeSize, shapeSize, Color(100, 255, 100, 255));
    renderer.drawText(Vec2(startX + spacing - 25, startY + shapeSize * 2 + 10), "Pentagon", Color(150, 150, 150, 255), 10);

    // Octagon
    drawOctagon(renderer, startX + spacing * 2, startY + shapeSize, shapeSize, Color(100, 100, 255, 255));
    renderer.drawText(Vec2(startX + spacing * 2 - 25, startY + shapeSize * 2 + 10), "Octagon", Color(150, 150, 150, 255), 10);

    // Diamond
    float dx = startX + spacing * 3;
    float dy = startY + shapeSize;
    renderer.drawRect(Rect(dx - shapeSize / 2, dy - shapeSize, shapeSize, shapeSize * 2), Color(0, 0, 0, 0)); // Clear area
    // Draw diamond manually
    for (float t = 0; t <= 1; t += 0.01f) {
        float halfW = shapeSize * 0.7f * (1 - std::abs(t - 0.5f) * 2);
        renderer.drawRect(Rect(dx - halfW, dy - shapeSize + t * shapeSize * 2, halfW * 2, 2), Color(255, 200, 100, 255));
    }
    renderer.drawText(Vec2(dx - 22, startY + shapeSize * 2 + 10), "Diamond", Color(150, 150, 150, 255), 10);

    // Row 2: Stars
    float row2Y = startY + 120;
    renderer.drawText(Vec2(x + 10, row2Y - 20), "Stars:", Color(180, 180, 180, 255), 12);

    // 5-point star
    drawStar(renderer, startX, row2Y + shapeSize, shapeSize, shapeSize * 0.4f, 5, Color(255, 220, 50, 255));
    renderer.drawText(Vec2(startX - 22, row2Y + shapeSize * 2 + 10), "5-Star", Color(150, 150, 150, 255), 10);

    // 6-point star
    drawStar(renderer, startX + spacing, row2Y + shapeSize, shapeSize, shapeSize * 0.5f, 6, Color(220, 50, 255, 255));
    renderer.drawText(Vec2(startX + spacing - 22, row2Y + shapeSize * 2 + 10), "6-Star", Color(150, 150, 150, 255), 10);

    // 8-point star
    drawStar(renderer, startX + spacing * 2, row2Y + shapeSize, shapeSize, shapeSize * 0.5f, 8, Color(50, 220, 255, 255));
    renderer.drawText(Vec2(startX + spacing * 2 - 22, row2Y + shapeSize * 2 + 10), "8-Star", Color(150, 150, 150, 255), 10);

    // Triangle
    float tx = startX + spacing * 3;
    float ty = row2Y + shapeSize * 2;
    for (float row = 0; row < shapeSize * 2; row += 2) {
        float width = (row / (shapeSize * 2)) * shapeSize * 1.5f;
        renderer.drawRect(Rect(tx - width / 2, ty - row, width, 2), Color(100, 255, 200, 255));
    }
    renderer.drawText(Vec2(tx - 22, row2Y + shapeSize * 2 + 10), "Triangle", Color(150, 150, 150, 255), 10);

    // Row 3: Progress bar demo
    float row3Y = row2Y + 120;
    renderer.drawText(Vec2(x + 10, row3Y - 20), "Progress:", Color(180, 180, 180, 255), 12);

    // Animated progress bar
    Rect progressTrack(x + 20, row3Y, width - 40, 20);
    renderer.drawRect(progressTrack, Color(40, 40, 50, 255));

    float progressWidth = progressTrack.width * g_app.progressValue;
    Rect progressFill(progressTrack.x, progressTrack.y, progressWidth, progressTrack.height);

    // Gradient-like effect
    for (float px = 0; px < progressWidth; px += 2) {
        float ratio = px / progressTrack.width;
        uint8_t r = static_cast<uint8_t>(80 + ratio * 100);
        uint8_t g = static_cast<uint8_t>(160 - ratio * 60);
        uint8_t b = static_cast<uint8_t>(80 + ratio * 80);
        renderer.drawRect(Rect(progressTrack.x + px, progressTrack.y, 2, progressTrack.height), Color(r, g, b, 255));
    }

    char progressText[32];
    snprintf(progressText, sizeof(progressText), "%.0f%%", g_app.progressValue * 100);
    renderer.drawText(Vec2(x + width / 2 - 15, row3Y + 3), progressText, Color(255, 255, 255, 255), 12);
}

// ============================================================================
// Retained Mode Demo
// ============================================================================

void initRetainedModeDemo() {
    g_app.retainedUI = std::make_unique<UIContainer>("retained_demo");
    g_app.retainedUI->setBounds(Rect(0, 0, 400, 500));
    g_app.retainedUI->setLayoutDirection(LayoutDirection::Vertical);
    g_app.retainedUI->setSpacing(10);
    g_app.retainedUI->setPadding(Insets(15));

    // Add widgets
    auto& title = g_app.retainedUI->addWidget<Label>("Retained Mode Widgets");

    auto& btn1 = g_app.retainedUI->addWidget<Button>("Primary Action");
    btn1.setOnClick([](const WidgetEvent& e) { printf("Primary button clicked!\n"); });

    auto& btn2 = g_app.retainedUI->addWidget<Button>("Secondary Action");

    auto& checkbox = g_app.retainedUI->addWidget<Checkbox>("Dark Mode");
    checkbox.setOnValueChanged([](const WidgetEvent& e) { printf("Checkbox toggled!\n"); });

    auto& slider = g_app.retainedUI->addWidget<Slider>(0.0f, 100.0f, 50.0f);
    slider.setLabel("Brightness");

    auto& input = g_app.retainedUI->addWidget<TextInput>();
    input.setPlaceholder("Enter your name...");

    // Panel with nested content
    auto& panel = g_app.retainedUI->addWidget<Panel>("Settings Panel");
    panel.addChild<Label>("Nested label inside panel");
    panel.addChild<Button>("Nested Button");
}

void renderRetainedModeDemo(SoftwareRenderer& renderer, float x, float y, float width, float height) {
    // Panel background
    renderer.drawRect(Rect(x, y, width, height), Color(30, 35, 30, 240));
    renderer.drawRectOutline(Rect(x, y, width, height), Color(60, 70, 60, 255), 2);

    // Update bounds and layout
    g_app.retainedUI->setBounds(Rect(x, y, width, height));
    g_app.retainedUI->layout();

    // Render each widget manually for software renderer
    float contentY = y + 15;
    float padding = 15;

    // Title
    renderer.drawText(Vec2(x + padding, contentY), "Retained Mode Widgets", Color(255, 255, 255, 255), 16);
    contentY += 30;

    // Iterate widgets and render
    for (const auto& widget : g_app.retainedUI->getRoot()->getChildren()) {
        Rect bounds(x + padding, contentY, width - 2 * padding, 30);

        if (auto* label = dynamic_cast<Label*>(widget.get())) {
            renderer.drawText(Vec2(bounds.x, bounds.y + 5), label->getText().c_str(), Color(200, 200, 200, 255));
            contentY += 25;
        } else if (auto* button = dynamic_cast<Button*>(widget.get())) {
            Color btnColor = button->isHovered() ? Color(70, 130, 70, 255) : Color(50, 110, 50, 255);
            renderer.drawRect(bounds, btnColor);
            renderer.drawText(Vec2(bounds.x + 10, bounds.y + 8), button->getLabel().c_str(), Color(255, 255, 255, 255));
            contentY += 40;
        } else if (auto* checkbox = dynamic_cast<Checkbox*>(widget.get())) {
            Rect boxRect(bounds.x, bounds.y + 5, 18, 18);
            renderer.drawRect(boxRect, Color(50, 60, 50, 255));
            renderer.drawRectOutline(boxRect, Color(100, 120, 100, 255));
            if (checkbox->isChecked()) {
                renderer.drawRect(Rect(boxRect.x + 4, boxRect.y + 4, 10, 10), Color(100, 200, 100, 255));
            }
            renderer.drawText(Vec2(boxRect.x + 25, bounds.y + 5), checkbox->getLabel().c_str(), Color(200, 200, 200, 255));
            contentY += 30;
        } else if (auto* slider = dynamic_cast<Slider*>(widget.get())) {
            renderer.drawText(Vec2(bounds.x, bounds.y), slider->getLabel().c_str(), Color(180, 180, 180, 255));
            contentY += 18;

            Rect trackRect(bounds.x, contentY, bounds.width, 8);
            renderer.drawRect(trackRect, Color(40, 50, 40, 255));

            float norm = (slider->getValue() - slider->getMin()) / (slider->getMax() - slider->getMin());
            float fillWidth = trackRect.width * norm;
            renderer.drawRect(Rect(trackRect.x, trackRect.y, fillWidth, trackRect.height), Color(80, 160, 80, 255));
            contentY += 25;
        } else if (auto* input = dynamic_cast<TextInput*>(widget.get())) {
            Rect inputRect(bounds.x, bounds.y, bounds.width, 28);
            renderer.drawRect(inputRect, Color(35, 45, 35, 255));
            renderer.drawRectOutline(inputRect, Color(80, 100, 80, 255));

            const char* displayText = input->getText().empty() ? input->getPlaceholder().c_str() : input->getText().c_str();
            Color textColor = input->getText().empty() ? Color(100, 100, 100, 255) : Color(220, 220, 220, 255);
            renderer.drawText(Vec2(inputRect.x + 5, inputRect.y + 6), displayText, textColor);
            contentY += 40;
        } else if (auto* panel = dynamic_cast<Panel*>(widget.get())) {
            Rect panelRect(bounds.x, bounds.y, bounds.width, 80);
            renderer.drawRect(panelRect, Color(40, 50, 40, 255));
            renderer.drawRectOutline(panelRect, Color(70, 90, 70, 255));

            // Panel header
            renderer.drawRect(Rect(panelRect.x, panelRect.y, panelRect.width, 24), Color(50, 60, 50, 255));
            renderer.drawText(Vec2(panelRect.x + 8, panelRect.y + 4), panel->getTitle().c_str(), Color(220, 220, 220, 255));

            contentY += 90;
        } else {
            contentY += 30;
        }
    }
}

// ============================================================================
// Main Render Loop
// ============================================================================

void updateAnimations(float deltaTime) {
    // Update progress bar animation
    if (g_app.progressForward) {
        g_app.progressValue += deltaTime * 0.3f;
        if (g_app.progressValue >= 1.0f) {
            g_app.progressValue = 1.0f;
            g_app.progressForward = false;
        }
    } else {
        g_app.progressValue -= deltaTime * 0.3f;
        if (g_app.progressValue <= 0.0f) {
            g_app.progressValue = 0.0f;
            g_app.progressForward = true;
        }
    }

    // Update shape rotation
    g_app.shapeRotation += deltaTime * 45.0f; // 45 degrees per second
    if (g_app.shapeRotation >= 360.0f) {
        g_app.shapeRotation -= 360.0f;
    }
}

void render() {
    g_renderer->clear(Color(25, 25, 30, 255));

    float panelWidth = 400;
    float panelHeight = 400;
    float padding = 15;

    // Header
    g_renderer->drawText(Vec2(20, 15), "DaktLib-GUI Showcase", Color(255, 255, 255, 255), 20);
    g_renderer->drawText(Vec2(20, 40), "Press TAB/Shift+TAB/Ctrl+TAB to toggle panels", Color(150, 150, 150, 255), 12);
    g_renderer->drawText(Vec2(20, 55), "Press 1/2/3 to switch tabs, ESC to exit", Color(150, 150, 150, 255), 12);

    float startY = 75;
    float currentX = padding;

    // Immediate Mode Demo
    if (g_app.showImmediateDemo) {
        renderImmediateModeDemo(*g_renderer, currentX, startY, panelWidth, panelHeight);
        currentX += panelWidth + padding;
    }

    // Retained Mode Demo
    if (g_app.showRetainedDemo) {
        renderRetainedModeDemo(*g_renderer, currentX, startY, panelWidth, panelHeight);
        currentX += panelWidth + padding;
    }

    // Shapes Demo
    if (g_app.showShapesDemo) {
        renderShapesDemo(*g_renderer, currentX, startY, panelWidth, panelHeight);
    }

    // Status bar
    float statusY = g_app.windowHeight - 25;
    g_renderer->drawRect(Rect(0, statusY, g_app.windowWidth, 25), Color(35, 35, 40, 255));

    char statusText[256];
    snprintf(statusText, sizeof(statusText), "Window: %dx%d | Slider: %.1f%% | Tab: %d | Progress: %.0f%% | Panels: [%c] Imm [%c] Ret [%c] Shapes", g_app.windowWidth, g_app.windowHeight, g_app.sliderValue * 100, g_app.selectedTab + 1,
             g_app.progressValue * 100, g_app.showImmediateDemo ? 'X' : ' ', g_app.showRetainedDemo ? 'X' : ' ', g_app.showShapesDemo ? 'X' : ' ');
    g_renderer->drawText(Vec2(10, statusY + 5), statusText, Color(150, 150, 150, 255), 12);
}

// ============================================================================
// Main Entry Point
// ============================================================================

static GLuint g_texture = 0;

void initOpenGL() {
    glGenTextures(1, &g_texture);
    glBindTexture(GL_TEXTURE_2D, g_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void displayFramebuffer(int width, int height, const uint8_t* pixels) {
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    // Upload pixel data to texture
    glBindTexture(GL_TEXTURE_2D, g_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Setup orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw fullscreen quad with texture
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    glTexCoord2f(1, 0);
    glVertex2f(width, 0);
    glTexCoord2f(1, 1);
    glVertex2f(width, height);
    glTexCoord2f(0, 1);
    glVertex2f(0, height);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

int main(int argc, char** argv) {
    printf("DaktLib-GUI Showcase Example\n");
    printf("============================\n\n");

    // Initialize GLFW
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    // Create window with OpenGL context for displaying framebuffer
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(g_app.windowWidth, g_app.windowHeight, "DaktLib-GUI Showcase", nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync
    glfwSetFramebufferSizeCallback(window, glfwFramebufferSizeCallback);
    glfwSetKeyCallback(window, glfwKeyCallback);

    // Initialize OpenGL texture for display
    initOpenGL();

    // Initialize software renderer
    g_renderer = std::make_unique<SoftwareRenderer>(g_app.windowWidth, g_app.windowHeight);

    // Initialize retained mode demo
    initRetainedModeDemo();

    printf("Window created successfully!\n");
    printf("Controls:\n");
    printf("  TAB       - Toggle Immediate Mode demo\n");
    printf("  Shift+TAB - Toggle Retained Mode demo\n");
    printf("  1/2/3     - Switch tabs\n");
    printf("  ESC       - Exit\n\n");

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Handle mouse input for demo interactivity
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        bool leftButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        // Track dragging state for sliders
        static bool wasPressed = false;
        static bool draggingSlider = false;
        static bool draggingProgressBar = false;

        // Slider area bounds
        float checkboxY = 80 + 40 + 40;
        float sliderY = checkboxY + 50;
        float sliderX = 30;
        float sliderWidth = 340;
        float sliderHeight = 16;

        // Check if starting a drag on slider
        if (leftButton && !wasPressed) {
            // Check if clicking checkbox area (immediate mode panel)
            if (mx >= 30 && mx <= 50 && my >= checkboxY && my <= checkboxY + 18) {
                g_app.checkboxValue = !g_app.checkboxValue;
                printf("Checkbox toggled: %s\n", g_app.checkboxValue ? "ON" : "OFF");
            }

            // Check slider area - start drag
            if (mx >= sliderX && mx <= sliderX + sliderWidth && my >= sliderY && my <= sliderY + sliderHeight) {
                draggingSlider = true;
            }

            // Check tab area
            float tabY = sliderY + 60;
            if (my >= tabY && my <= tabY + 28) {
                float tabWidth = 350.0f / 3;
                int tabIndex = static_cast<int>((mx - 30) / tabWidth);
                if (tabIndex >= 0 && tabIndex < 3) {
                    g_app.selectedTab = tabIndex;
                    printf("Tab selected: %d\n", tabIndex + 1);
                }
            }
        }

        // Handle slider dragging - update value while mouse is held
        if (leftButton && draggingSlider) {
            float newValue = static_cast<float>((mx - sliderX) / sliderWidth);
            g_app.sliderValue = std::max(0.0f, std::min(1.0f, newValue));
        }

        // Stop dragging when mouse released
        if (!leftButton) {
            if (draggingSlider) {
                printf("Slider value: %.1f%%\n", g_app.sliderValue * 100);
            }
            draggingSlider = false;
            draggingProgressBar = false;
        }

        wasPressed = leftButton;

        // Calculate delta time
        static double lastTime = glfwGetTime();
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // Update animations
        updateAnimations(deltaTime);

        // Render to software framebuffer
        render();

        // Display the framebuffer using OpenGL
        displayFramebuffer(g_app.windowWidth, g_app.windowHeight, g_renderer->getPixels());

        glfwSwapBuffers(window);
    }

    // Cleanup
    glDeleteTextures(1, &g_texture);
    g_renderer.reset();
    g_app.retainedUI.reset();

    glfwDestroyWindow(window);
    glfwTerminate();

    printf("\nShowcase completed successfully!\n");
    return 0;
}
