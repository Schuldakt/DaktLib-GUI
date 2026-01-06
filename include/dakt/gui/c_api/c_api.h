#ifndef DAKTLIB_GUI_C_API_H
#define DAKTLIB_GUI_C_API_H

#include <cstdint>
#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t DaktGUI_Context;

DaktGUI_Context daktlib_gui_create();
void daktlib_gui_destroy(DaktGUI_Context ctx);

void daktlib_gui_new_frame(DaktGUI_Context ctx, float delta_time);
void daktlib_gui_render(DaktGUI_Context ctx);

#ifdef __cplusplus
}
#endif

#endif
