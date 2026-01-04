#pragma once

#ifdef _WIN32
#if defined(DAKT_GUI_BUILD_SHARED)
#define DAKT_GUI_API __declspec(dllexport)
#elif defined(DAKT_GUI_USE_SHARED)
#define DAKT_GUI_API __declspec(dllimport)
#else
#define DAKT_GUI_API
#endif
#else
#define DAKT_GUI_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DaktGuiContext *DaktGuiContextHandle;
typedef struct DaktGuiBackend *DaktGuiBackendHandle;

typedef struct {
  float x;
  float y;
  float width;
  float height;
} DaktGuiRect;

typedef struct {
  int key;
  int down;
} DaktGuiKeyEvent;

typedef struct {
  int button;
  int down;
  float x;
  float y;
} DaktGuiMouseEvent;

typedef struct {
  const char *text;
} DaktGuiTextEvent;

DAKT_GUI_API DaktGuiContextHandle dakt_gui_create(DaktGuiBackendHandle backend);
DAKT_GUI_API void dakt_gui_destroy(DaktGuiContextHandle ctx);

DAKT_GUI_API void dakt_gui_new_frame(DaktGuiContextHandle ctx, float dt);
DAKT_GUI_API void dakt_gui_process_mouse(DaktGuiContextHandle ctx,
                                         const DaktGuiMouseEvent *ev);
DAKT_GUI_API void dakt_gui_process_key(DaktGuiContextHandle ctx,
                                       const DaktGuiKeyEvent *ev);
DAKT_GUI_API void dakt_gui_process_text(DaktGuiContextHandle ctx,
                                        const DaktGuiTextEvent *ev);

DAKT_GUI_API int dakt_gui_begin_window(DaktGuiContextHandle ctx,
                                       const char *title, DaktGuiRect rect,
                                       int flags);
DAKT_GUI_API void dakt_gui_end_window(DaktGuiContextHandle ctx);
DAKT_GUI_API int dakt_gui_button(DaktGuiContextHandle ctx, const char *label);
DAKT_GUI_API int dakt_gui_checkbox(DaktGuiContextHandle ctx, const char *label,
                                   int *value);
DAKT_GUI_API int dakt_gui_slider_float(DaktGuiContextHandle ctx,
                                       const char *label, float *v, float min,
                                       float max);

DAKT_GUI_API void dakt_gui_render(DaktGuiContextHandle ctx);

#ifdef __cplusplus
}
#endif
