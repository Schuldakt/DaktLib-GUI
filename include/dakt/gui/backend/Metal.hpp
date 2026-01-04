#pragma once

#include "IRenderBackend.hpp"

namespace dakt::gui::backend {

class Metal : public IRenderBackend {
public:
  Metal() = default;
  void createResources() override {}
  void beginFrame() override {}
  void submit(const DrawList &) override {}
  void present() override {}
  void resize(int, int) override {}
  void shutdown() override {}
};

} // namespace dakt::gui::backend
