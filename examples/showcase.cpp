#include "dakt/gui/GUI.hpp"
#include <cstdio>

using namespace dakt::gui;

class NullBackend final : public IRenderBackend {
public:
    void createResources() override {}
    void beginFrame() override {}
    void submit(const DrawList&) override {}
    void present() override {}
    void resize(int, int) override {}
    void shutdown() override {}
};

int main() {
    NullBackend backend;
    Context ctx{&backend};
    Immediate ui{&ctx};
    FrameTiming timing{1.0F / 60.0F};
    Frame frame{timing};
    DrawList draw;

    // This is just a stub showcase; replace with real per-frame UI once widgets land.
    backend.createResources();
    backend.beginFrame();
    backend.submit(draw);
    backend.present();
    backend.shutdown();

    std::puts("DaktLib-GUI showcase stub ran.");
    return 0;
}
