#ifndef DAKT_GUI_IMMEDIATE_HPP
#define DAKT_GUI_IMMEDIATE_HPP

namespace dakt::gui {

class Context;

void beginFrame(Context* ctx, float deltaTime);
void endFrame(Context* ctx);

} // namespace dakt::gui

#endif
