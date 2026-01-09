#include "dakt/gui/immediate/Widgets/Color.hpp"

#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/immediate/Containers/Layout.hpp"

#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"
#include "dakt/gui/immediate/Widgets/Button.hpp"
#include "dakt/gui/immediate/Widgets/Text.hpp"

#include <cstring>

namespace dakt::gui {


    bool colorEdit3(const char* label, Color* color) {
        if (!color)
            return false;
        text("%s", label);
        sameLine();
        return colorButton("##color", *color);
    }

    bool colorEdit4(const char* label, Color* color) {
        return colorEdit3(label, color);
    }

    bool colorPicker3(const char* label, Color* color) {
        Context* ctx = getCurrentContext();
        if (!ctx || !color) return false;

        ImmediateState& s = getState();
        if (!s.currentWindow) return false;
        if (s.currentWindow->skipItems) return false;

        // Note: This is a simplified version - full implementation would need
        // proper state management for dragging, etc.
        text("%s Color Picker", label);
        text("R:%d G:%d B:%d", color->r, color->g, color->b);
        
        return false;
    }

    bool colorPicker4(const char* label, Color* color) {
        return colorPicker3(label, color);
    }
} // namespace dakt::gui