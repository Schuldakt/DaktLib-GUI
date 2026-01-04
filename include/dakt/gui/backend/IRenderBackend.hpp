#pragma once

#include "../core/Types.hpp"

namespace dakt::gui {

class DrawList;

class IRenderBackend {
public:
  virtual ~IRenderBackend() = default;
  virtual void createResources() = 0;
  virtual void beginFrame() = 0;
  virtual void submit(const DrawList &) = 0;
  virtual void present() = 0;
  virtual void resize(int width, int height) = 0;
  virtual void shutdown() = 0;
};

} // namespace dakt::gui
