#ifndef DAKTLIB_GUI_HIT_TEST_HPP
#define DAKTLIB_GUI_HIT_TEST_HPP

#include "../../core/Types.hpp"
#include "../layout/Layout.hpp"
#include <cstdint>
#include <vector>

namespace dakt::gui {

struct HitTestResult {
    uint64_t widgetID = 0;
    LayoutNode* node = nullptr;
    int zOrder = 0; // Higher z-order wins
    bool hit = false;
};

class HitTester {
  public:
    HitTester();
    ~HitTester();

    // Perform hit test at point
    HitTestResult testPoint(LayoutNode* root, const Vec2& point);

    // Test multiple points (multi-selection, drag)
    std::vector<HitTestResult> testPoints(LayoutNode* root, const std::vector<Vec2>& points);

    // Test rectangle (selection box)
    std::vector<HitTestResult> testRect(LayoutNode* root, const Rect& area);

  private:
    HitTestResult recursiveTest(LayoutNode* node, const Vec2& point, int zOrder);
    void recursiveRectTest(LayoutNode* node, const Rect& area, std::vector<HitTestResult>& results, int zOrder);
};

} // namespace dakt::gui

#endif
