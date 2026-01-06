#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"

namespace dakt::gui {

    thread_local ImmediateState* g_current = nullptr;

    void setCurrent(ImmediateState* state) { g_current = state; }

    ImmediateState& current() { return *g_current; }

    bool hasCurrent() { return g_current != nullptr; }

} // namespace dakt::gui