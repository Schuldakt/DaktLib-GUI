#ifndef DAKT_GUI_DRAW_HPP
#define DAKT_GUI_DRAW_HPP

#include "../core/Types.hpp"
#include <vector>

namespace dakt::gui {

class DrawList {
  public:
    DrawList();
    ~DrawList();

    void reset();
    void drawRect(const Rect& rect, Color color);

  private:
    std::vector<uint8_t> commands_;
};

} // namespace dakt::gui

#endif
