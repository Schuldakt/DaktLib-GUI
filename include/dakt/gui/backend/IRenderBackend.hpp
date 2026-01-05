#ifndef DAKT_GUI_IRENDER_BACKEND_HPP
#define DAKT_GUI_IRENDER_BACKEND_HPP

#include <cstdint>

namespace dakt::gui {

class DrawList;

class IRenderBackend {
  public:
    virtual ~IRenderBackend() = default;

    virtual bool createResources() = 0;
    virtual void beginFrame() = 0;
    virtual void submit(const DrawList& drawList) = 0;
    virtual void present() = 0;
    virtual void resize(uint32_t width, uint32_t height) = 0;
    virtual void shutdown() = 0;

    virtual const char* getName() const = 0;
};

} // namespace dakt::gui

#endif // DAKT_GUI_IRENDER_BACKEND_HPP
