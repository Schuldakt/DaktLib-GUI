#ifndef DAKTLIB_GUI_CONTEXT_HPP
#define DAKTLIB_GUI_CONTEXT_HPP

#include "../subsystems/style/Style.hpp"
#include "Types.hpp"
#include <memory>
#include <vector>

namespace dakt::gui {

class IRenderBackend;
class DrawList;
class LayoutNode;
class InputSystem;

struct ImmediateState;

class Context {
  public:
    Context(IRenderBackend* backend);
    ~Context();

    void newFrame(float deltaTime);
    void endFrame();

    // State accessors
    float getDeltaTime() const { return deltaTime_; }
    uint32_t getFrameCount() const { return frameCount_; }
    IRenderBackend* getBackend() { return backend_; }
    Theme& getTheme() { return theme_; }
    const Theme& getTheme() const { return theme_; }

    // Layout & rendering
    DrawList& getDrawList();
    LayoutNode* getRootLayout();

    // Immediate state
    ImmediateState& getImmediateState();
    const ImmediateState& getImmediateState() const;

  private:
    IRenderBackend* backend_;
    Theme theme_;
    float deltaTime_ = 0.0f;
    uint32_t frameCount_ = 0;
    std::unique_ptr<DrawList> drawList_;
    std::unique_ptr<LayoutNode> rootLayout_;
    std::unique_ptr<ImmediateState> immediateState_;
};

} // namespace dakt::gui

#endif // DAKTLIB_GUI_CONTEXT_HPP
