#pragma once

#include "Types.hpp"

namespace dakt::gui {

class IRenderBackend;

class Context {
public:
  Context() = default;
  explicit Context(IRenderBackend *backend) : backend_(backend) {}
  ~Context() = default;

  void setBackend(IRenderBackend *backend) { backend_ = backend; }
  IRenderBackend *backend() const { return backend_; }

private:
  IRenderBackend *backend_{nullptr};
};

} // namespace dakt::gui
