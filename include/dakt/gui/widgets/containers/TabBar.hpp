// ============================================================================
// DaktLib GUI Module - Tab Bar Container
// ============================================================================

#pragma once

#include <dakt/core/Types.hpp>
#include <dakt/gui/core/Types.hpp>

namespace dakt::gui
{

using namespace dakt::core;

// ============================================================================
// Tab Bar Flags
// ============================================================================

enum class TabBarFlags : u32
{
    None = 0,
    Reorderable = 1 << 0,
    AutoSelectNewTabs = 1 << 1,
    TabListPopupButton = 1 << 2,
    NoCloseWithMiddleMouseButton = 1 << 3,
    NoTabListScrollingButtons = 1 << 4,
    NoTooltip = 1 << 5,
    FittingPolicyResizeDown = 1 << 6,
    FittingPolicyScroll = 1 << 7,
};

inline TabBarFlags operator|(TabBarFlags a, TabBarFlags b)
{
    return static_cast<TabBarFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

// ============================================================================
// Tab Item Flags
// ============================================================================

enum class TabItemFlags : u32
{
    None = 0,
    UnsavedDocument = 1 << 0,
    SetSelected = 1 << 1,
    NoCloseWithMiddleMouseButton = 1 << 2,
    NoPushId = 1 << 3,
    NoTooltip = 1 << 4,
    NoReorder = 1 << 5,
    Leading = 1 << 6,
    Trailing = 1 << 7,
};

inline TabItemFlags operator|(TabItemFlags a, TabItemFlags b)
{
    return static_cast<TabItemFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

// ============================================================================
// Tab Bar Functions
// ============================================================================

// Begin a tab bar
bool beginTabBar(StringView id, TabBarFlags flags = TabBarFlags::None);
void endTabBar();

// Begin a tab item (returns true if the tab is selected)
bool beginTabItem(StringView label, bool* open = nullptr, TabItemFlags flags = TabItemFlags::None);
void endTabItem();

// Set a tab as selected
void setTabItemClosed(StringView tabOrDockedWindowLabel);

}  // namespace dakt::gui
