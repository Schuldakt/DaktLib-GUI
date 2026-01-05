// DrawList implementation stubs
#include "dakt/gui/draw/DrawList.hpp"

namespace dakt::gui {

DrawList::DrawList() = default;
DrawList::~DrawList() = default;

void DrawList::reset() { commands_.clear(); }

void DrawList::drawRect(const Rect& rect, Color color) {
    // Draw a rectangle
}

void DrawBatcher_Init() {}
} // namespace dakt::gui
