#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/backend/IRenderBackend.hpp"
#include "dakt/gui/subsystems/draw/DrawList.hpp"
#include "dakt/gui/subsystems/layout/Layout.hpp"
#include "dakt/gui/immediate/internal/ImmediateState.hpp"


namespace dakt::gui {

    Context::Context(IRenderBackend* backend) : backend_(backend), deltaTime_(0.0f), frameCount_(0), drawList_(std::make_unique<DrawList>()), rootLayout_(std::make_unique<LayoutNode>()) {}

    Context::~Context() = default;

    void Context::newFrame(float deltaTime) {
        deltaTime_ = deltaTime;
        frameCount_++;
        drawList_->reset();
        immediateState_ = std::make_unique<ImmediateState>();
    }

    void Context::endFrame() {
        // Frame processing
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
