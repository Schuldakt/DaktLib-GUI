#pragma once

#include "IRenderBackend.hpp"

namespace dakt::gui::backend {

class Vulkan : public IRenderBackend {
public:
  Vulkan() = default;
  void createResources() override {}
  void beginFrame() override {}
  void submit(const DrawList &) override {}
  void present() override {}
  void resize(int, int) override {}
  void shutdown() override {}
};

} // namespace dakt::gui::backend
