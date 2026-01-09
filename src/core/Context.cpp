#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/backend/IRenderBackend.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"
#include "dakt/gui/subsystems/layout/Layout.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"


namespace dakt::gui {

    Context::Context(IRenderBackend* backend)
        : backend_(backend)
        , deltaTime_(0.0f)
        , frameCount_(0)
        , drawList_(std::make_unique<DrawList>())
        , rootLayout_(std::make_unique<LayoutNode>())
        , immediateState_(std::make_unique<ImmediateState>())  // Create once, persist across frames
    {}

    Context::~Context() = default;

    void Context::newFrame(float deltaTime) {
        deltaTime_ = deltaTime;
        frameCount_++;
        drawList_->reset();
        // Don't recreate immediateState_ - it persists across frames
    }

    void Context::endFrame() {
        // Frame processing
    }

    void Context::beginInputFrame() {
        // Copy current buttons into prevButtons
        for (int i = 0; i < static_cast<int>(MouseButton::COUNT); i++) {
            mouseInput_.prevButtons[i] = mouseInput_.buttons[i];
        }

        // Clear per-frame accumulators
        mouseInput_.delta = Vec2(0, 0);
        mouseInput_.wheelDelta = 0.0f;
    }

    void Context::setMousePosition(const Vec2& pos) {
        mouseInput_.position = pos;
    }

    void Context::setMouseDelta(const Vec2& delta) {
        mouseInput_.delta = delta;
    }

    void Context::addMouseWheelDelta(float wheelDelta) {
        mouseInput_.wheelDelta += wheelDelta;
    }

    void Context::setMouseButton(MouseButton button, bool down) {
        const int idx = static_cast<int>(button);
        if (idx < 0 || idx >= static_cast<int>(MouseButton::COUNT)) {
            return;
        }
        mouseInput_.buttons[idx] = down;
    }

    void Context::setMouseInput(const MouseInput& input) {
        mouseInput_ = input;
    }

    const MouseInput& Context::getMouseInput() const {
        return mouseInput_;
    }

    void Context::setKeyboardInput(const KeyboardInput& input) {
        keyboardInput_ = input;
    }

    const KeyboardInput& Context::getKeyboardInput() const {
        return keyboardInput_;
    }

    ImmediateState& Context::getImmediateState() {
        return *immediateState_;
    }
    const ImmediateState& Context::getImmediateState() const {
        return *immediateState_;
    }

    DrawList& Context::getDrawList() { return *drawList_; }

    LayoutNode* Context::getRootLayout() { return rootLayout_.get(); }

} // namespace dakt::gui
