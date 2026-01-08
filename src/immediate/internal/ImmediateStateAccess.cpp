#include "dakt/gui/immediate/internal/ImmediateStateAccess.hpp"
#include "dakt/gui/core/Context.hpp"

namespace dakt::gui {

    // Thread-local storage for current context
    static thread_local Context* g_currentContext = nullptr;

    void setCurrentContext(Context* ctx) {
        g_currentContext = ctx;
    }

    Context* getCurrentContext() {
        return g_currentContext;
    }

    bool hasCurrentContext() {
        return g_currentContext != nullptr;
    }

    ImmediateState& getState() {
        return g_currentContext->getImmediateState();
    }

    bool hasState() {
        return g_currentContext != nullptr;
    }

} // namespace dakt::gui
