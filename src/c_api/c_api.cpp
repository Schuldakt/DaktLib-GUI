#include "dakt/gui/c_api/c_api.h"
#include "dakt/gui/backend/IRenderBackend.hpp"
#include "dakt/gui/core/Context.hpp"

using dakt::gui::Context;
using dakt::gui::IRenderBackend;

extern "C" {

struct DaktGuiContext {
  Context context;
};

struct DaktGuiBackend {
  IRenderBackend *impl{nullptr};
};

DAKT_GUI_API DaktGuiContextHandle
dakt_gui_create(DaktGuiBackendHandle backend) {
  DaktGuiContext *ctx = new DaktGuiContext{};
  if (backend) {
    ctx->context.setBackend(backend->impl);
  }
  return ctx;
}

DAKT_GUI_API void dakt_gui_destroy(DaktGuiContextHandle ctx) { delete ctx; }

DAKT_GUI_API void dakt_gui_new_frame(DaktGuiContextHandle, float) {}

DAKT_GUI_API void dakt_gui_process_mouse(DaktGuiContextHandle,
                                         const DaktGuiMouseEvent *) {}

DAKT_GUI_API void dakt_gui_process_key(DaktGuiContextHandle,
                                       const DaktGuiKeyEvent *) {}

DAKT_GUI_API void dakt_gui_process_text(DaktGuiContextHandle,
                                        const DaktGuiTextEvent *) {}

DAKT_GUI_API int dakt_gui_begin_window(DaktGuiContextHandle, const char *,
                                       DaktGuiRect, int) {
  return 0;
}

DAKT_GUI_API void dakt_gui_end_window(DaktGuiContextHandle) {}

DAKT_GUI_API int dakt_gui_button(DaktGuiContextHandle, const char *) {
  return 0;
}

DAKT_GUI_API int dakt_gui_checkbox(DaktGuiContextHandle, const char *, int *) {
  return 0;
}

DAKT_GUI_API int dakt_gui_slider_float(DaktGuiContextHandle, const char *,
                                       float *, float, float) {
  return 0;
}

DAKT_GUI_API void dakt_gui_render(DaktGuiContextHandle) {}

} // extern "C"
