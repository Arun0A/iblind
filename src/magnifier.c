#include "magnifier.h"
#include "globals.h"
#include <stdlib.h>
#include <stdio.h>

void update_zoom_dims(void) {
    if (ratio < 1.0f) ratio = 1.0f;
    capture_width = (int)(box_width / ratio);
    if (capture_width < 4) capture_width = 4;
    capture_height = (int)(box_height / ratio);
    if (capture_height < 4) capture_height = 4;

    half_capture_width = capture_width / 2;
    half_capture_height = capture_height / 2;

    free(map_src_word_x);
    free(map_src_word_y);

    map_src_word_x = malloc(box_width * sizeof(uint32_t));
    map_src_word_y = malloc(box_height * sizeof(uint32_t));

    for (int dx = 0; dx < box_width; dx++) {
        int sx = (int)(dx / ratio);
        if (sx >= capture_width) sx = capture_width - 1;
        map_src_word_x[dx] = sx;
    }

    for (int dy = 0; dy < box_height; dy++) {
        int sy = (int)(dy / ratio);
        if (sy >= capture_height) sy = capture_height - 1;
        map_src_word_y[dy] = sy * capture_width;
    }
}

void do_image(void) {
    // In persist mode, we don't get global mouse events so we query the mouse coordinates
    if (persist_mode) {
        Window g;
        int i;
        uint32_t m = 0;
        XQueryPointer(
            dpy, root, &g, &target,
            &x, &y, &i, &i, &m
        );

        bool mouse_in_original = (x >= base_x && x < base_x + win_width &&
                                   y >= base_y && y < base_y + win_height);
        if (mouse_in_original) {
            if (!shifted) {
                shifted = true;
                XMoveWindow(dpy, win, opposite_x, opposite_y);
            }
        } else {
            if (shifted) {
                shifted = false;
                XMoveWindow(dpy, win, base_x, base_y);
            }
        }
    }

    int lx = x - half_capture_width;
    int ly = y - half_capture_height;

    if (lx < 0) lx = 0;
    if (ly < 0) ly = 0;

    if (lx + capture_width > dpy_width) {
        lx = dpy_width - capture_width;
    }
    if (ly + capture_height > dpy_height) {
        ly = dpy_height - capture_height;
    }
    if (lx < 0) lx = 0;
    if (ly < 0) ly = 0;

    XImage *img = XGetImage(
        dpy, root,
        lx, ly, capture_width, capture_height,
        AllPlanes, ZPixmap
    );

    if (!img) return;

    uint32_t *dest32 = (uint32_t *)image_data;
    uint32_t *src32 = (uint32_t *)img->data;

    for (int dy = 0; dy < box_height; dy++) {
        uint32_t src_row = map_src_word_y[dy];
        uint32_t dest_row = dy * box_width;
        for (int dx = 0; dx < box_width; dx++) {
            dest32[dest_row + dx] = src32[src_row + map_src_word_x[dx]];
        }
    }

    XImage *img_2 = XCreateImage(
        dpy, visual, depth, ZPixmap, 0,
        (char *)image_data, box_width, box_height, 32, 0
    );

    XPutImage(dpy, win, gc, img_2, 0, 0, padding_val, padding_val, box_width, box_height);

    XDestroyImage(img);
    img_2->data = NULL;
    XDestroyImage(img_2);
}
