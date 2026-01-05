#ifndef DAKT_GUI_STYLE_HPP
#define DAKT_GUI_STYLE_HPP

#include "../core/Types.hpp"

namespace dakt::gui {

class Theme {
  public:
    Theme();
    ~Theme();

    void setColor(const char* name, Color color);
    Color getColor(const char* name) const;

  private:
    Color colors_[16] = {};
};

} // namespace dakt::gui

#endif
