#ifndef DAKTLIB_GUI_IMMEDIATECONTEXT_HPP
#define DAKTLIB_GUI_IMMEDIATECONTEXT_HPP

namespace dakt::gui {

// ============================================================================
// Layout
// ============================================================================

void sameLine(float offsetX = 0.0f, float spacing = -1.0f);
void newLine();
void spacing();
void separator();
void indent(float width = 0.0f);
void unindent(float width = 0.0f);

void beginGroup();
void endGroup();

} // namespace dakt::gui

#endif