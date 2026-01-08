#include "dakt/gui/core/Types.hpp"
#include "dakt/gui/subsystems/input/HitTest.hpp"
#include "dakt/gui/subsystems/input/Input.hpp"
#include "dakt/gui/subsystems/layout/Layout.hpp"
#include <cassert>
#include <iostream>
#include <vector>


using namespace dakt::gui;

// ============================================================================
// Type System Tests
// ============================================================================

void test_vec2_operations() {
    std::cout << "Testing Vec2 operations..." << std::endl;

    Vec2 a(3.0f, 4.0f);
    Vec2 b(1.0f, 2.0f);

    // Length
    assert(a.length() > 4.99f && a.length() < 5.01f); // 5.0

    // Addition
    Vec2 sum = a + b;
    assert(sum.x == 4.0f && sum.y == 6.0f);

    // Subtraction
    Vec2 diff = a - b;
    assert(diff.x == 2.0f && diff.y == 2.0f);

    // Scalar multiplication
    Vec2 scaled = a * 2.0f;
    assert(scaled.x == 6.0f && scaled.y == 8.0f);

    // Dot product
    float dot = a.dot(b);
    assert(dot == 11.0f); // 3*1 + 4*2 = 11

    std::cout << "✓ Vec2 operations passed" << std::endl;
}

void test_rect_geometry() {
    std::cout << "Testing Rect geometry..." << std::endl;

    Rect r(10.0f, 20.0f, 100.0f, 50.0f);

    // Accessors
    assert(r.right() == 110.0f);
    assert(r.bottom() == 70.0f);
    assert(r.center().x == 60.0f && r.center().y == 45.0f);

    // Point containment
    assert(r.contains(Vec2(50.0f, 40.0f)));
    assert(!r.contains(Vec2(5.0f, 25.0f)));
    assert(!r.contains(Vec2(120.0f, 40.0f)));

    // Rectangle intersection
    Rect r2(50.0f, 30.0f, 80.0f, 100.0f);
    assert(r.intersects(r2));

    // Intersection bounds
    Rect intersection = r.intersection(r2);
    assert(intersection.x == 50.0f);
    assert(intersection.y == 30.0f);
    assert(intersection.right() == 110.0f);
    assert(intersection.bottom() == 70.0f);

    std::cout << "✓ Rect geometry passed" << std::endl;
}

void test_color_operations() {
    std::cout << "Testing Color operations..." << std::endl;

    Color c(255, 128, 64, 255);

    // RGB values
    assert(c.r == 255);
    assert(c.g == 128);
    assert(c.b == 64);
    assert(c.a == 255);

    // RGBA encoding
    uint32_t rgba = c.toRGBA();
    Color decoded(rgba);
    assert(decoded.r == c.r);
    assert(decoded.g == c.g);
    assert(decoded.b == c.b);

    // HSV conversion
    Color hsv = Color::fromHSV(0.0f, 1.0f, 1.0f); // Red
    assert(hsv.r == 255);

    std::cout << "✓ Color operations passed" << std::endl;
}

// ============================================================================
// Layout System Tests
// ============================================================================

void test_layout_node_basic() {
    std::cout << "Testing LayoutNode basic operations..." << std::endl;

    LayoutNode node;
    node.setSize(100.0f, 50.0f);
    node.setPosition(10.0f, 20.0f);

    Rect rect = node.getRect();
    assert(rect.x == 10.0f);
    assert(rect.y == 20.0f);
    assert(rect.width == 100.0f);
    assert(rect.height == 50.0f);

    Vec2 size = node.getSize();
    assert(size.x == 100.0f && size.y == 50.0f);

    std::cout << "✓ LayoutNode basic operations passed" << std::endl;
}

void test_layout_hierarchy() {
    std::cout << "Testing LayoutNode hierarchy..." << std::endl;

    LayoutNode root;
    root.setSize(200.0f, 200.0f);

    LayoutNode* child1 = root.addChild();
    assert(child1 != nullptr);
    assert(child1->getParent() == &root);

    LayoutNode* child2 = root.addChild();
    assert(child2 != nullptr);

    const auto& children = root.getChildren();
    assert(children.size() == 2);

    root.removeChild(child1);
    assert(root.getChildren().size() == 1);

    std::cout << "✓ LayoutNode hierarchy passed" << std::endl;
}

void test_flex_properties() {
    std::cout << "Testing Flex properties..." << std::endl;

    LayoutNode node;

    FlexProperties props;
    props.direction = FlexDirection::Column;
    props.grow = 1.0f;
    props.shrink = 0.5f;

    node.setFlexProperties(props);

    const auto& retrieved = node.getFlexProperties();
    assert(retrieved.direction == FlexDirection::Column);
    assert(retrieved.grow == 1.0f);
    assert(retrieved.shrink == 0.5f);

    std::cout << "✓ Flex properties passed" << std::endl;
}

void test_layout_dirty_tracking() {
    std::cout << "Testing layout dirty tracking..." << std::endl;

    LayoutNode root;
    root.markClean();
    assert(!root.isDirty());

    root.markDirty();
    assert(root.isDirty());

    std::cout << "✓ Layout dirty tracking passed" << std::endl;
}

// ============================================================================
// Input System Tests
// ============================================================================

void test_input_mouse_state() {
    std::cout << "Testing Input mouse state..." << std::endl;

    InputSystem input;

    MouseInput mouseInput;
    mouseInput.position = Vec2(100.0f, 50.0f);
    mouseInput.buttons[0] = true; // Left button

    input.processMouse(mouseInput);

    assert(input.getMousePosition().x == 100.0f);
    assert(input.isMouseButtonDown(MouseButton::Left));

    std::cout << "✓ Input mouse state passed" << std::endl;
}

void test_input_keyboard_state() {
    std::cout << "Testing Input keyboard state..." << std::endl;

    InputSystem input;

    KeyboardInput kbInput;
    kbInput.keyPressed[static_cast<int>(Key::A)] = true;
    kbInput.textInput = "hello";

    input.processKeyboard(kbInput);

    assert(input.isKeyDown(Key::A));
    assert(input.getTextInput() == "hello");

    std::cout << "✓ Input keyboard state passed" << std::endl;
}

void test_input_frame_state_reset() {
    std::cout << "Testing Input frame state reset..." << std::endl;

    InputSystem input;

    MouseInput mouseInput;
    mouseInput.wheelDelta = 5.0f;
    input.processMouse(mouseInput);

    assert(input.getMouseWheel() == 5.0f);

    input.resetFrameState();
    assert(input.getMouseWheel() == 0.0f);

    std::cout << "✓ Input frame state reset passed" << std::endl;
}

void test_input_button_transitions() {
    std::cout << "Testing Input button transitions..." << std::endl;

    InputSystem input;

    // Initial: button not pressed
    assert(!input.isMouseButtonDown(MouseButton::Left));

    // Press button
    MouseInput m1;
    m1.buttons[0] = true;
    input.processMouse(m1);

    assert(input.isMouseButtonPressed(MouseButton::Left));
    assert(input.isMouseButtonDown(MouseButton::Left));

    // Hold button
    MouseInput m2;
    m2.buttons[0] = true;
    input.processMouse(m2);

    assert(!input.isMouseButtonPressed(MouseButton::Left)); // Not just pressed
    assert(input.isMouseButtonDown(MouseButton::Left));     // Still down

    // Release button
    MouseInput m3;
    m3.buttons[0] = false;
    input.processMouse(m3);

    assert(input.isMouseButtonReleased(MouseButton::Left));
    assert(!input.isMouseButtonDown(MouseButton::Left));

    std::cout << "✓ Input button transitions passed" << std::endl;
}

// ============================================================================
// Hit Testing Tests
// ============================================================================

void test_hittest_point() {
    std::cout << "Testing HitTest point detection..." << std::endl;

    LayoutNode root;
    root.setPosition(0.0f, 0.0f);
    root.setSize(200.0f, 200.0f);

    LayoutNode* child = root.addChild();
    child->setPosition(50.0f, 50.0f);
    child->setSize(100.0f, 100.0f);

    HitTester tester;

    // Hit parent only
    HitTestResult result1 = tester.testPoint(&root, Vec2(25.0f, 25.0f));
    assert(result1.hit);
    assert(result1.node == &root);

    // Hit child (should be returned instead of parent)
    HitTestResult result2 = tester.testPoint(&root, Vec2(75.0f, 75.0f));
    assert(result2.hit);
    assert(result2.node == child);
    assert(result2.zOrder > 0); // Child has higher z-order

    // Miss all
    HitTestResult result3 = tester.testPoint(&root, Vec2(250.0f, 250.0f));
    assert(!result3.hit);

    std::cout << "✓ HitTest point detection passed" << std::endl;
}

void test_hittest_rectangle() {
    std::cout << "Testing HitTest rectangle detection..." << std::endl;

    LayoutNode root;
    root.setPosition(0.0f, 0.0f);
    root.setSize(200.0f, 200.0f);

    LayoutNode* child = root.addChild();
    child->setPosition(50.0f, 50.0f);
    child->setSize(100.0f, 100.0f);

    HitTester tester;

    // Test area covering both
    Rect testArea(30.0f, 30.0f, 150.0f, 150.0f);
    auto results = tester.testRect(&root, testArea);

    // Should hit both parent and child
    assert(results.size() >= 2);

    std::cout << "✓ HitTest rectangle detection passed" << std::endl;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    std::cout << "\n======== DaktLib-GUI Phase 1 Unit Tests ========\n" << std::endl;

    try {
        // Type system
        test_vec2_operations();
        test_rect_geometry();
        test_color_operations();

        // Layout
        test_layout_node_basic();
        test_layout_hierarchy();
        test_flex_properties();
        test_layout_dirty_tracking();

        // Input
        test_input_mouse_state();
        test_input_keyboard_state();
        test_input_frame_state_reset();
        test_input_button_transitions();

        // Hit testing
        test_hittest_point();
        test_hittest_rectangle();

        std::cout << "\n======== ✓ All tests passed! ========\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "✗ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
