#pragma once

#include "IRenderBackend.hpp"

namespace dakt::gui::backend {

class DX11 : public IRenderBackend {
public:
  DX11() = default;
  void createResources() override {}
  void beginFrame() override {}
  void submit(const DrawList &) override {}
  void present() override {}
  void resize(int, int) override {}
  void shutdown() override {}
};

} // namespace dakt::gui::backend
