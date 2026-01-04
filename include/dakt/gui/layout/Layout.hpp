#pragma once

#include "../core/Types.hpp"

namespace dakt::gui {

class LayoutNode {
public:
  LayoutNode() = default;
  virtual ~LayoutNode() = default;
};

class FlexLayout {
public:
  FlexLayout() = default;
  void setRoot(LayoutNode *node) { root_ = node; }
  LayoutNode *root() const { return root_; }

private:
  LayoutNode *root_{nullptr};
};

} // namespace dakt::gui
