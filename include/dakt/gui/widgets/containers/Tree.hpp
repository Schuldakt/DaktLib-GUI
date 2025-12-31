// ============================================================================
// DaktLib GUI Module - Tree Container
// ============================================================================

#pragma once

#include <dakt/core/Types.hpp>
#include <dakt/gui/core/Types.hpp>

namespace dakt::gui
{

using namespace dakt::core;

// ============================================================================
// Tree Node Flags
// ============================================================================

enum class TreeNodeFlags : u32
{
    None = 0,
    Selected = 1 << 0,
    Framed = 1 << 1,
    AllowItemOverlap = 1 << 2,
    NoTreePushOnOpen = 1 << 3,
    NoAutoOpenOnLog = 1 << 4,
    DefaultOpen = 1 << 5,
    OpenOnDoubleClick = 1 << 6,
    OpenOnArrow = 1 << 7,
    Leaf = 1 << 8,
    Bullet = 1 << 9,
    FramePadding = 1 << 10,
    SpanAvailWidth = 1 << 11,
    SpanFullWidth = 1 << 12,
    NavLeftJumpsBackHere = 1 << 13,
    CollapsingHeader = Framed | NoTreePushOnOpen | NoAutoOpenOnLog,
};

inline TreeNodeFlags operator|(TreeNodeFlags a, TreeNodeFlags b)
{
    return static_cast<TreeNodeFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline bool hasFlag(TreeNodeFlags flags, TreeNodeFlags flag)
{
    return (static_cast<u32>(flags) & static_cast<u32>(flag)) != 0;
}

// ============================================================================
// Tree Functions
// ============================================================================

// Tree nodes (expandable/collapsible)
bool treeNode(StringView label, TreeNodeFlags flags = TreeNodeFlags::None);
bool treeNode(StringView strId, StringView label, TreeNodeFlags flags = TreeNodeFlags::None);
bool treeNodeEx(StringView label, TreeNodeFlags flags = TreeNodeFlags::None);
void treePush(StringView strId);
void treePop();
f32 getTreeNodeToLabelSpacing();

// Collapsing headers (tree node without indentation)
bool collapsingHeader(StringView label, TreeNodeFlags flags = TreeNodeFlags::None);
bool collapsingHeader(StringView label, bool* open, TreeNodeFlags flags = TreeNodeFlags::None);

}  // namespace dakt::gui
