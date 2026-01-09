#include "dakt/gui/immediate/core/ImmediateContext.hpp"

#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/immediate/ImmediateContext.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"

#include "dakt/gui/immediate/internal/ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"

#include <cstdint>
#include <cstring>

namespace dakt::gui {

    // Hasing helpers
    static std::uint64_t fnv1a64(const void* data, std::size_t len, std::uint64_t seed = 14695981039346656037ULL) {
        const std::uint8_t* bytes = static_cast<const std::uint8_t*>(data);
        std::uint64_t hash = seed;
        for (std::size_t i = 0; i < len; ++i) {
            hash ^= bytes[i];
            hash *= 1099511628211ULL;
        }
        return hash;
    }

    static std::uint64_t hashString(const char* str, std::uint64_t seed) {
        if (!str) return seed;
        return fnv1a64(str, std::strlen(str), seed);
    }

    static std::uint64_t hashPtr(const void* ptr, std::uint64_t seed) {
        std::uint64_t v = reinterpret_cast<std::uintptr_t>(ptr);
        return fnv1a64(&v, sizeof(v), seed);
    }

    static std::uint64_t currentIdSeed(ImmediateState& s) {
        // If not stack, use a fixed seed.
        // If stack exists, combine from the top.
        if (s.idStack.empty()) return 1465981039346656037ULL;
        return static_cast<std::uint64_t>(s.idStack.back());
    }

    // ID stack API
    void pushID(const char* strId) {
        ImmediateState& s = getState();
        std::uint64_t seed = currentIdSeed(s);
        std::uint64_t h = hashString(strId, seed);
        s.idStack.push_back(static_cast<ID>(h));
    }

    void pushID(ID id) {
        ImmediateState& s = getState();
        // Combine numeric ID with current seed
        std::int64_t seed = currentIdSeed(s);
        std::uint64_t h = fnv1a64(&id, sizeof(id), seed);
        s.idStack.push_back(static_cast<ID>(h));
    }

    void popID() {
        ImmediateState& s = getState();
        if (!s.idStack.empty()) {
            s.idStack.pop_back();
        }
    }

    ID getID(const char* strId) {
        ImmediateState& s = getState();
        std::uint64_t seed = currentIdSeed(s);
        return static_cast<ID>(hashString(strId, seed));
    }

    ID getID(const void* ptr) {
        ImmediateState& s = getState();
        std::uint64_t seed = currentIdSeed(s);
        return static_cast<ID>(hashPtr(ptr, seed));
    }

    // Mouse utilties
    Vec2 getMousePos() {
        ImmediateState& s = getState();
        return s.mouse.position;
    }

    bool isMouseDown(MouseButton button) {
        ImmediateState& s = getState();
        int idx = static_cast<int>(button);
        if (idx < 0 || idx >= static_cast<int>(MouseButton::COUNT)) return false;
        return s.mouse.buttons[idx];
    }

    bool isMouseClicked(MouseButton button) {
        ImmediateState& s = getState();
        int idx = static_cast<int>(button);
        if (idx < 0 || idx >= static_cast<int>(MouseButton::COUNT)) return false;
        return s.mouse.buttons[idx] && !s.mouse.prevButtons[idx];
    }

    bool isMouseReleased(MouseButton button) {
        ImmediateState& s = getState();
        int idx = static_cast<int>(button);
        if (idx < 0 || idx >= static_cast<int>(MouseButton::COUNT)) return false;
        return !s.mouse.buttons[idx] && s.mouse.prevButtons[idx];
    }

    // Item query utilities
    bool isItemHovered() {
        ImmediateState& s = getState();
        return s.lastItemHovered;
    }

    bool isItemActive() {
        ImmediateState& s = getState();
        return s.lastItemActive;
    }

    bool isItemClicked() {
        ImmediateState& s = getState();
        return s.lastItemClicked;
    }

    Vec2 getItemRectMin() {
        ImmediateState s = getState();
        return Vec2(s.lastItemRect.x, s.lastItemRect.y);
    }

    Vec2 getItemRectMax() {
        ImmediateState& s = getState();
        return Vec2(s.lastItemRect.x + s.lastItemRect.width, s.lastItemRect.y + s.lastItemRect.height);
    }

    Vec2 getItemRectSize() {
        ImmediateState& s = getState();
        return Vec2(s.lastItemRect.width, s.lastItemRect.height);
    }

    // DrawList Access
    DrawList* getWindowDrawList() {
        Context* ctx = getCurrentContext();
        if (!ctx) return nullptr;
        return &ctx->getDrawList();
    }

} // namespace dakt::gui