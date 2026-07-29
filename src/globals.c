#include "globals.h"
#include <stddef.h>

bool running = true;

int screen;
int dpy_width;
int dpy_height;
Visual *visual;
int depth;
Window win;
Window root;
Display *dpy;
GC gc;

int x = 0;
int y = 0;

int padding_val = 2;
int margin_val = 5;
unsigned long border_color = 0x9C9C9C;
bool persist_mode = true;

int box_width = 800;
int box_height = 450;
int win_width;
int win_height;

float ratio = 2.0f;
int capture_width;
int capture_height;
int half_capture_width;
int half_capture_height;

int base_x;
int base_y;
int opposite_x;
int opposite_y;
bool shifted = false;

Window target;

uint8_t *image_data = NULL;
uint32_t *map_src_word_x = NULL;
uint32_t *map_src_word_y = NULL;
