#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "dakt/gui/core/Types.hpp"
namespace dakt::gui {

// Forward declarations to avoid heavy includes.
class Context;
class DrawList;

struct Vec2;
struct Rect;

using Id = std::uint32_t;

struct ItemState {
    bool hovered = false;
    bool active = false;
    bool focused = false;

    bool pressed = false;
    bool clicked = false;
    bool released = false;
};

struct WindowState {
    std::string title;

    Vec2 pos{};
    Vec2 size{};

    Rect rect{};
    Rect contentRect{};

    Vec2 cursor{};
    Vec2 cursorStart{};

    float lineHeight = 0.0f;
    float indent = 0.0f;

    bool open = true;
    bool focused = false;

    DrawList* drawList = nullptr;
};

struct GroupState {
    Vec2 cursorBackup{};
    float indentBackup = 0.0f;
    float lineHeightBackup = 0.0f;
};

struct ImmediateState {
    Context* ctx = nullptr;

    float deltaTime = 0.0f;
    std::uint64_t frameIndex = 0;

    // ID stack
    std::vector<ID> idStack;

    // Windows
    std::vector<WindowState> windowStack;
    std::unordered_map<Id, WindowState> windowsById;

    // Last item state (for queries like isItemHovered())
    ItemState lastItem{};

    // Per-item persistent states (hover/active etc.)
    std::unordered_map<Id, ItemState> itemStates;

    // Next-window settings (set by setNextWindowX calls)
    std::optional<Vec2> nextWindowPos;
    std::optional<Vec2> nextWindowSize;
    std::optional<bool> nextWindowCollapsed;

    // Temporary storage for group stacks
    struct GroupState {
        Vec2 cursorBackup{};
        float indentBackup = 0.0f;
        float lineHeightBackup = 0.0f;
    };
    std::vector<GroupState> groupStack;
};

// Global singleton state for now (you can later refactor into a context-owned instance).
extern ImmediateState g_state;

} // namespace dakt::gui