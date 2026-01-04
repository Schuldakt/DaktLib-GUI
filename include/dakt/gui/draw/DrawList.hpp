#pragma once

#include "../core/Types.hpp"
#include <cstddef>
#include <vector>

namespace dakt::gui {

struct Vertex {
  Vec2 pos{};
  Vec2 uv{};
  Color color{};
};

struct DrawCommand {
  Rect clip{};
  std::size_t elemCount{0};
};

class DrawList {
public:
  DrawList() = default;
  void clear() {
    vertices_.clear();
    commands_.clear();
  }
  const std::vector<Vertex> &vertices() const { return vertices_; }
  const std::vector<DrawCommand> &commands() const { return commands_; }

private:
  std::vector<Vertex> vertices_{};
  std::vector<DrawCommand> commands_{};
};

} // namespace dakt::gui
