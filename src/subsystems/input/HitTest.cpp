// HitTest implementation stubs
#include "dakt/gui/subsystems/input/HitTest.hpp"
#include <algorithm>

namespace dakt::gui {

HitTester::HitTester() = default;
HitTester::~HitTester() = default;

HitTestResult HitTester::testPoint(LayoutNode* root, const Vec2& point) {
    if (!root)
        return HitTestResult();
    return recursiveTest(root, point, 0);
}

HitTestResult HitTester::recursiveTest(LayoutNode* node, const Vec2& point, int zOrder) {
    HitTestResult result;

    // Check if point is within this node's bounds
    Rect nodeRect = node->getRect();
    if (!nodeRect.contains(point)) {
        return result;
    }

    // This node is hit - keep traversing children to find deepest hit
    result.node = node;
    result.zOrder = zOrder;
    result.hit = true;

    // Test children (they have higher z-order)
    const auto& children = node->getChildren();
    int childZOrder = zOrder + 1;

    // Traverse children in reverse order (last child is topmost)
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        HitTestResult childResult = recursiveTest(it->get(), point, childZOrder);
        if (childResult.hit && childResult.zOrder > result.zOrder) {
            result = childResult;
        }
        childZOrder++;
    }

    return result;
}

std::vector<HitTestResult> HitTester::testPoints(LayoutNode* root, const std::vector<Vec2>& points) {
    std::vector<HitTestResult> results;
    for (const auto& point : points) {
        results.push_back(testPoint(root, point));
    }
    return results;
}

std::vector<HitTestResult> HitTester::testRect(LayoutNode* root, const Rect& area) {
    std::vector<HitTestResult> results;
    if (!root)
        return results;

    recursiveRectTest(root, area, results, 0);

    // Sort by z-order (descending)
    std::sort(results.begin(), results.end(), [](const HitTestResult& a, const HitTestResult& b) { return a.zOrder > b.zOrder; });

    return results;
}

void HitTester::recursiveRectTest(LayoutNode* node, const Rect& area, std::vector<HitTestResult>& results, int zOrder) {
    if (!node)
        return;

    Rect nodeRect = node->getRect();

    // Check if rectangles intersect
    if (nodeRect.intersects(area)) {
        HitTestResult result;
        result.node = node;
        result.zOrder = zOrder;
        result.hit = true;
        results.push_back(result);
    }

    // Test children
    const auto& children = node->getChildren();
    for (const auto& child : children) {
        recursiveRectTest(child.get(), area, results, zOrder + 1);
    }
}

} // namespace dakt::gui
