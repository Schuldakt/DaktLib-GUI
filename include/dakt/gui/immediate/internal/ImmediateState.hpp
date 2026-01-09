#pragma once

#include "dakt/gui/core/Types.hpp"

#include <vector>
#include <memory>
#include <unordered_map>

namespace dakt::gui {

    class Context;
    class DrawList;
    class InputSystem;

    // Window state
    struct WindowState {
        ID id = 0;
        const char* name = nullptr;
        Vec2 pos;
        Vec2 size;
        Vec2 contentSize;
        Vec2 cursorPos;
        Vec2 cursorStartPos;
        WindowFlags flags = WindowFlags::None;
        bool collapsed = false;
        bool skipItems = false;
    };

    struct GroupState {
        Vec2 cursorPosBackup{};
        Vec2 cursorStartBackup{};
        Rect lastItemRectBackup{};
        float indentBackup = 0.0f;
    };

    struct ChildState {
        WindowState* parentWindow = nullptr;
        Vec2 parentCursorBackup{};
        Vec2 parentCursorStartBackup{};
        Rect parentLastItemRectBackup{};
        Vec2 childPos{};
        Vec2 childSize{};
        WindowState* childWindow = nullptr;
    };

    struct ColorPickerState {
        bool initialized = false;   // Track if HSV has been initialized from color
        bool draggingSV = false;
        bool draggingHue = false;
        bool draggingAlpha = false;
        float hue = 0;
        float sat = 1;
        float val = 1;
    };

    struct MenuState {
        bool mainMenuBarOpen = false;
        bool menuBarOpen = false;
        std::vector<const char*> menuStack;
        Vec2 menuPosition;
        float menuBarHeight = 28.0f;
        int hoveredMenuItem = -1;
        std::vector<Rect> menuItemRects;
    };

    struct PopUpState {
        std::vector<const char*> popupStack;
        std::vector<Vec2> popupPositions;
        const char* pendingPopup = nullptr;
        bool closeRequested = false;
    };

    struct TooltipState {
        bool tooltipActive = false;
        std::string tooltipText;
        Vec2 tooltipPos;
    };

    struct TableState {
        bool tableActive = false;
        const char* tableId = nullptr;
        int columns = 0;
        int currentColumn = -1;
        int currentRow = -1;
        TableFlags flags = TableFlags::None;
        Vec2 tablePos;
        Vec2 tableSize;
        std::vector<float> columnWidths;
        std::vector<const char*> columnLabels;
        float rowHeight = 24.0f;
        float headerHeight = 28.0f;
    };

    // Immediate State (owned by Context)
    struct ImmediateState {
        float deltaTime = 0.0f;

        // Frame counter
        std::uint64_t frameIndex = 0;
        
        // ID stack
        std::vector<ID> idStack;
        ID currentId = 0;

        // Persistent window storage (stable pointers)
        std::unordered_map<ID, std::unique_ptr<WindowState>> windowsById;

        // Pointer-based window stack (no copies)
        std::vector<WindowState*> windowStack;
        WindowState* currentWindow = nullptr;

        // GroupState stack
        std::vector<GroupState> groupStack;

        // ChildState stack
        std::vector<ChildState> childStack;

        // Next window hints
        bool nextWindowPosSet = false;
        Vec2 nextWindowPos;
        bool nextWindowSizeSet = false;
        Vec2 nextWindowSize;

        // Last item state (for queries like isItemHovered())
        ID lastItemId = 0;
        Rect lastItemRect;
        bool lastItemHovered = false;
        bool lastItemActive = false;
        bool lastItemClicked = false;
        bool lastItemEdited = false;
        bool lastItemActivated = false;
        bool lastItemDeactivated = false;

        // Mouse state (captured each frame)
        MouseInput mouse{};

        // Hot/Active tracking
        ID hotId = 0;
        ID activeId = 0;
        ID prevActiveId = 0;
        
        // Focus management
        ID focusRequestId = 0;
        int focusRequestOffset = 0;
        ID keyboardFocusId = 0;
        ID defaultFocusId = 0;

        // Double-click tracking
        float lastClickTime[5] = {};
        Vec2 lastClickPos[5] = {};
        ID lastClicked[5] = {};
        static constexpr float DOUBLE_CLICK_TIME = 0.3f;
        static constexpr float DOUBLE_CLICK_DIST = 6.0f;

        // Input system reference
        InputSystem* input = nullptr;

        // Accumulated time
        float totalTime = 0.0f;

        // Widget-specific color state
        std::unordered_map<ID, ColorPickerState> colorPickerStates;

        // Menu state
        MenuState menuState;

        // PopUp state
        PopUpState popupState;

        // Tooltip state
        TooltipState tooltipState;

        // Table state
        TableState tableState;
    };

} // namespace dakt::gui