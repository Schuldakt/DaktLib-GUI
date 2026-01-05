#include "dakt/gui/core/Context.hpp"
#include "dakt/gui/backend/IRenderBackend.hpp"
#include "dakt/gui/draw/DrawList.hpp"
#include "dakt/gui/layout/Layout.hpp"


namespace dakt::gui {

Context::Context(IRenderBackend* backend) : backend_(backend), deltaTime_(0.0f), frameCount_(0), drawList_(std::make_unique<DrawList>()), rootLayout_(std::make_unique<LayoutNode>()) {}

Context::~Context() = default;

void Context::newFrame(float deltaTime) {
    deltaTime_ = deltaTime;
    frameCount_++;
    drawList_->reset();
}

void Context::endFrame() {
    // Frame processing
}

DrawList& Context::getDrawList() { return *drawList_; }

LayoutNode* Context::getRootLayout() { return rootLayout_.get(); }

} // namespace dakt::gui
