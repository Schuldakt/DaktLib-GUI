#pragma once

#include "dakt/gui/core/Types.hpp"

#include <cstdint>

namespace dakt::gui {

    inline TreeNodeFlags operator|(TreeNodeFlags a, TreeNodeFlags b) {
        return static_cast<TreeNodeFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    DAKTLIB_GUI_API bool treeNode(const char* label, TreeNodeFlags flags = TreeNodeFlags::None);
    DAKTLIB_GUI_API bool treeNodeEx(const char* strId, TreeNodeFlags flags, const char* fmt, ...);
    DAKTLIB_GUI_API void treePop();

    DAKTLIB_GUI_API bool collapsingHeader(const char* label, TreeNodeFlags flags = TreeNodeFlags::None);

} // namespace dakt::gui