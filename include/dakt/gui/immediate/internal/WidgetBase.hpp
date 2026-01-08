#pragma once

#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/core/Types.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"
#include "dakt/gui/subsystems/input/Input.hpp"
#include "dakt/gui/subsystems/style/Style.hpp"
#include "dakt/gui/subsystems/text/Text.hpp"
#include "dakt/gui/subsystems/layout/Layout.hpp"
#include "dakt/gui/immediate/Containers/Window.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

namespace dakt::gui {

    // ========================================================================
    // Widget Setup Helper - Reduces boilerplate in widget implementations
    // ========================================================================

    struct WidgetSetup {
        Context* ctx;
        ImmediateState* state;
        DrawList* dl;
        const ColorScheme* colors;

        // Check if setup is valid
        explicit operator bool() const { return ctx != nullptr; }

        // Convenience accessors
        ImmediateState& getState() const { return *state; }
        DrawList& getDrawList() const { return *dl; }
        const ColorScheme& getColors() const { return *colors; }
    };

    // Returns invalid WidgetSetup if context not available
    inline WidgetSetup widgetSetup() {
        Context* ctx = getCurrentContext();
        if (!ctx) return {nullptr, nullptr, nullptr, nullptr};

        return {
            ctx,
            &getState(),
            &ctx->getDrawList(),
            &ctx->getTheme().colors()
        };
    }

    // ========================================================================
    // Convenience accessors for widget implementations
    // ========================================================================

    // Get draw list from current context
    inline DrawList& getDrawList() {
        return getCurrentContext()->getDrawList();
    }

    // Get color scheme from current context
    inline const ColorScheme& getColors() {
        return getCurrentContext()->getTheme().colors();
    }

    // ========================================================================
    // Widget Helper: Update Item State
    // ========================================================================

    inline void updateItemState(ID id, bool hovered, bool active) {
        ImmediateState& state = getState();

        bool wasActive = (state.prevActiveId == id);
        state.lastItemActivated = active && !wasActive;
        state.lastItemDeactivated = wasActive && !active;

        if (active) {
            state.activeId = id;
        } else if (state.activeId == id) {
            state.activeId = 0;
        }

        state.lastItemId = id;
        state.lastItemHovered = hovered;
        state.lastItemActive = active;
    }

} // namespace dakt::gui
