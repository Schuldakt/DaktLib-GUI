#ifndef DAKT_GUI_RETAINED_HPP
#define DAKT_GUI_RETAINED_HPP

namespace dakt::gui {

class UIContainer {
  public:
    UIContainer();
    ~UIContainer();
};

class Widget {
  public:
    Widget();
    virtual ~Widget();
};

} // namespace dakt::gui

#endif
