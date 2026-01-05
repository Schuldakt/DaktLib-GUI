#ifndef DAKT_GUI_C_API_H
#define DAKT_GUI_C_API_H

#include <cstdint>
#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t DaktGUI_Context;

DaktGUI_Context dakt_gui_create();
void dakt_gui_destroy(DaktGUI_Context ctx);

void dakt_gui_new_frame(DaktGUI_Context ctx, float delta_time);
void dakt_gui_render(DaktGUI_Context ctx);

#ifdef __cplusplus
}
#endif

#endif
