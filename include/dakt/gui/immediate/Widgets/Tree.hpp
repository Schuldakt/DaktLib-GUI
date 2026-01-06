#pragma once

#include "dakt/gui/core/Types.hpp"

#include <cstdint>

namespace dakt::gui {

    enum class TreeNodeFlags : uint32_t {
        None                = 0,
        Selected            = 1 << 0,
        Framed              = 1 << 1,
        OpenOnArrow         = 1 << 2,
        OpenOnDoubleClick   = 1 << 3,
        Leaf                = 1 << 4,
        DefaultOpen         = 1 << 5,
        SpanAllColumns      = 1 << 6
    };

    inline TreeNodeFlags operator|(TreeNodeFlags a, TreeNodeFlags b) {
        return static_cast<TreeNodeFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    DAKTLIB_GUI_API bool treeNode(const char* label, TreeNodeFlags flags = TreeNodeFlags::None);
    DAKTLIB_GUI_API bool treeNodeEx(const char* strId, TreeNodeFlags flags, const char* fmt, ...);
    DAKTLIB_GUI_API void treePop();

    DAKTLIB_GUI_API bool collapsingHeader(const char* label, TreeNodeFlags flags = TreeNodeFlags::None);

} // namespace dakt::gui