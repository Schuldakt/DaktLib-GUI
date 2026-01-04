#include "dakt/gui/GUI.hpp"
#include <cassert>

using namespace dakt::gui;

class DummyBackend final : public IRenderBackend {
public:
    void createResources() override { created = true; }
    void beginFrame() override { began = true; }
    void submit(const DrawList& list) override { submitted = submitted || !list.vertices().empty(); }
    void present() override { presented = true; }
    void resize(int w, int h) override { lastResize[0] = w; lastResize[1] = h; }
    void shutdown() override { shut = true; }

    bool created{false};
    bool began{false};
    bool submitted{false};
    bool presented{false};
    bool shut{false};
    int lastResize[2]{0, 0};
};

int main() {
    DummyBackend backend;
    Context ctx{&backend};
    Immediate ui{&ctx};
    FrameTiming timing{0.016F};
    Frame frame{timing};

    assert(ctx.backend() == &backend);
    assert(ui.context() == &ctx);
    assert(frame.timing().deltaTime == 0.016F);

    DrawList draw;
    draw.clear();
    backend.createResources();
    backend.beginFrame();
    backend.submit(draw);
    backend.present();
    backend.resize(1280, 720);
    backend.shutdown();

    assert(backend.created);
    assert(backend.began);
    assert(backend.presented);
    assert(backend.shut);
    assert(backend.lastResize[0] == 1280 && backend.lastResize[1] == 720);

    InputSystem input;
    InputState state{};
    state.mousePos = {10.0F, 20.0F};
    state.mouseButtons[0] = true;
    input.setState(state);
    assert(input.state().mousePos.x == 10.0F);
    assert(input.state().mousePos.y == 20.0F);
    assert(input.state().mouseButtons[0]);

    TextShaper shaper;
    shaper.setFallbackEnabled(false);
    assert(shaper.fallbackEnabled() == false);

    return 0;
}
