#ifndef DAKTLIB_GUI_IMMEDIATECONTEXT_HPP
#define DAKTLIB_GUI_IMMEDIATECONTEXT_HPP

#include "../../core/Types.hpp"

namespace dakt::gui {

// ============================================================================
// Trees & Lists
// ============================================================================

enum class TreeNodeFlags : uint32_t {
    None = 0,
    Selected = 1 << 0,
    Framed = 1 << 1,
    OpenOnArrow = 1 << 2,
    OpenOnDoubleClick = 1 << 3,
    Leaf = 1 << 4,
    DefaultOpen = 1 << 5,
    SpanAllColumns = 1 << 6,
};

inline TreeNodeFlags operator|(TreeNodeFlags a, TreeNodeFlags b) { return static_cast<TreeNodeFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }

bool treeNode(const char* label, TreeNodeFlags flags = TreeNodeFlags::None);
bool treeNodeEx(const char* strId, TreeNodeFlags flags, const char* fmt, ...);
void treePop();

bool collapsingHeader(const char* label, TreeNodeFlags flags = TreeNodeFlags::None);

bool selectable(const char* label, bool selected = false, Vec2 size = Vec2(0, 0));
bool selectable(const char* label, bool* selected, Vec2 size = Vec2(0, 0));

bool listBox(const char* label, int* currentItem, const char* const items[], int itemCount, int heightInItems = -1);

} // namespace dakt::gui

#endif