#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdbool.h>
#include <stdint.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

extern bool running;

extern int screen;
extern int dpy_width;
extern int dpy_height;
extern Visual *visual;
extern int depth;
extern Window win;
extern Window root;
extern Display *dpy;
extern GC gc;

extern int x;
extern int y;

extern int padding_val;
extern int margin_val;
extern unsigned long border_color;
extern bool persist_mode;

extern int box_width;
extern int box_height;
extern int win_width;
extern int win_height;

extern float ratio;
extern int capture_width;
extern int capture_height;
extern int half_capture_width;
extern int half_capture_height;

extern int base_x;
extern int base_y;
extern int opposite_x;
extern int opposite_y;
extern bool shifted;

extern Window target;

extern uint8_t *image_data;
extern uint32_t *map_src_word_x;
extern uint32_t *map_src_word_y;

#endif
