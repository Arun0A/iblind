
/*
    Particle is a super simple screen magnifier for x11
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

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

#define padding 2

int box_width = 512;
int box_height = 512;
int win_width;
int win_height;

int ratio = 4;
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

void update_zoom_dims(void) {
    if (ratio < 1) ratio = 1;
    capture_width = (box_width + ratio - 1) / ratio;
    capture_height = (box_height + ratio - 1) / ratio;
    half_capture_width = capture_width / 2;
    half_capture_height = capture_height / 2;
}

void grab_key(Display *dpy, Window root, KeySym keysym) {
    KeyCode code = XKeysymToKeycode(dpy, keysym);
    if (code) {
        XGrabKey(dpy, code, 0, root, true, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, code, ShiftMask, root, true, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, code, LockMask, root, true, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, code, ShiftMask | LockMask, root, true, GrabModeAsync, GrabModeAsync);
    }
}

void do_image(void) {
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

    for (int dy = 0; dy < box_height; dy++) {
        int sy = dy / ratio;
        if (sy >= capture_height) sy = capture_height - 1;
        for (int dx = 0; dx < box_width; dx++) {
            int sx = dx / ratio;
            if (sx >= capture_width) sx = capture_width - 1;

            uint32_t dest_idx = (dy * box_width + dx) * 4;
            uint32_t src_idx = (sy * capture_width + sx) * 4;

            image_data[dest_idx + 0] = img->data[src_idx + 0];
            image_data[dest_idx + 1] = img->data[src_idx + 1];
            image_data[dest_idx + 2] = img->data[src_idx + 2];
        }
    }

    XImage *img_2 = XCreateImage(
        dpy, visual, depth, ZPixmap, 0,
        (char *)image_data, box_width, box_height, 32, 0
    );

    XPutImage(dpy, win, gc, img_2, 0, 0, padding, padding, box_width, box_height);

    XDestroyImage(img);
    img_2->data = NULL;
    XDestroyImage(img_2);
}

int main(int argc, char **argv) {
    int init_x = 0;
    int init_y = 0;
    bool has_init_x = false;
    bool has_init_y = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--width") == 0) {
            if (i + 1 < argc) {
                box_width = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Error: %s requires an argument\n", argv[i]);
                return -1;
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--height") == 0) {
            if (i + 1 < argc) {
                box_height = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Error: %s requires an argument\n", argv[i]);
                return -1;
            }
        } else if (strcmp(argv[i], "-x") == 0 || strcmp(argv[i], "--xpos") == 0) {
            if (i + 1 < argc) {
                init_x = atoi(argv[++i]);
                has_init_x = true;
            } else {
                fprintf(stderr, "Error: %s requires an argument\n", argv[i]);
                return -1;
            }
        } else if (strcmp(argv[i], "-y") == 0 || strcmp(argv[i], "--ypos") == 0) {
            if (i + 1 < argc) {
                init_y = atoi(argv[++i]);
                has_init_y = true;
            } else {
                fprintf(stderr, "Error: %s requires an argument\n", argv[i]);
                return -1;
            }
        } else if (strcmp(argv[i], "-z") == 0 || strcmp(argv[i], "--zoom") == 0) {
            if (i + 1 < argc) {
                ratio = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Error: %s requires an argument\n", argv[i]);
                return -1;
            }
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-?") == 0) {
            printf("Usage: particle [options]\n");
            printf("Options:\n");
            printf("  -w, --width <val>   Width of the magnifier box (default: 512)\n");
            printf("  -h, --height <val>  Height of the magnifier box (default: 512)\n");
            printf("  -x, --xpos <val>    Initial X position of the window\n");
            printf("  -y, --ypos <val>    Initial Y position of the window\n");
            printf("  -z, --zoom <val>    Default zoom level (default: 4)\n");
            printf("  --help              Show this help message\n");
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            fprintf(stderr, "Use --help to see available options\n");
            return -1;
        }
    }

    if (box_width < 16) box_width = 16;
    if (box_height < 16) box_height = 16;
    if (ratio < 1) ratio = 1;

    int max_zoom_w = box_width / 4;
    int max_zoom_h = box_height / 4;
    int max_zoom = max_zoom_w < max_zoom_h ? max_zoom_w : max_zoom_h;
    if (max_zoom < 1) max_zoom = 1;
    if (ratio > max_zoom) ratio = max_zoom;

    win_width = box_width + padding * 2;
    win_height = box_height + padding * 2;

    update_zoom_dims();

    image_data = malloc(box_width * box_height * 4);
    if (!image_data) {
        fprintf(stderr, "Error: failed to allocate memory for image data\n");
        return -1;
    }

    if ((dpy = XOpenDisplay(NULL)) == NULL) {
        fprintf(stderr, "Error: can't open display :(\n");
        free(image_data);
        return -1;
    }

    screen = DefaultScreen(dpy);
    dpy_width = DisplayWidth(dpy, screen);
    dpy_height = DisplayHeight(dpy, screen);
    visual = DefaultVisual(dpy, screen);
    depth = DefaultDepth(dpy, screen);
    root = DefaultRootWindow(dpy);

    base_x = has_init_x ? init_x : (dpy_width - (win_width + 10));
    base_y = has_init_y ? init_y : (dpy_height - (win_height + 10));

    if (base_x < 10) base_x = 10;
    if (base_y < 10) base_y = 10;
    if (base_x > dpy_width - win_width - 10) base_x = dpy_width - win_width - 10;
    if (base_y > dpy_height - win_height - 10) base_y = dpy_height - win_height - 10;

    opposite_x = dpy_width - win_width - base_x;
    opposite_y = dpy_height - win_height - base_y;

    if (opposite_x < 10) opposite_x = 10;
    if (opposite_y < 10) opposite_y = 10;
    if (opposite_x > dpy_width - win_width - 10) opposite_x = dpy_width - win_width - 10;
    if (opposite_y > dpy_height - win_height - 10) opposite_y = dpy_height - win_height - 10;

    win = XCreateSimpleWindow(
        dpy, root,
        base_x, base_y,
        win_width, win_height,
        0, 0, 0xDB6A0B
    );

    gc = XCreateGC(dpy, win, 0, NULL);

    XSizeHints sh;
    sh.min_width  = sh.max_width  = win_width;
    sh.min_height = sh.max_height = win_height;
    sh.flags = PMinSize | PMaxSize;
    XSetWMNormalHints(dpy, win, &sh);

    XStoreName(dpy, win, "Particle");
    XMapWindow(dpy, win);
    XSelectInput(dpy, win, KeyPressMask | ExposureMask);
    XFlush(dpy);

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
        shifted = true;
        XMoveWindow(dpy, win, opposite_x, opposite_y);
    }

    Cursor c = XCreateFontCursor(dpy, 2);

    XGrabPointer(
        dpy, root, true,
        PointerMotionMask | ButtonPressMask,
        GrabModeAsync, GrabModeAsync, None,
        c, CurrentTime
    );

    grab_key(dpy, root, XK_q);
    grab_key(dpy, root, XK_Q);
    grab_key(dpy, root, XK_Escape);
    grab_key(dpy, root, XK_w);
    grab_key(dpy, root, XK_W);
    grab_key(dpy, root, XK_a);
    grab_key(dpy, root, XK_A);
    grab_key(dpy, root, XK_s);
    grab_key(dpy, root, XK_S);
    grab_key(dpy, root, XK_d);
    grab_key(dpy, root, XK_D);
    grab_key(dpy, root, XK_Up);
    grab_key(dpy, root, XK_Down);
    grab_key(dpy, root, XK_Left);
    grab_key(dpy, root, XK_Right);
    grab_key(dpy, root, XK_plus);
    grab_key(dpy, root, XK_minus);
    grab_key(dpy, root, XK_equal);
    grab_key(dpy, root, XK_KP_Add);
    grab_key(dpy, root, XK_KP_Subtract);

    uint32_t n = 0;
    XEvent ev;
    XSync(dpy, false);
    while (running) {
        bool found = XCheckMaskEvent(
            dpy,
            (KeyPressMask | ExposureMask | 
             PointerMotionMask | ButtonPressMask),
            &ev
        );

        if (n > 20000) {
            n = 0;
            do_image();
        }

        if (!found) {
            n++;
            continue;
        }

        if (ev.type == KeyPress) {
            KeySym keysym = XLookupKeysym(&ev.xkey, 0);
            switch (keysym) {
                case XK_q:
                case XK_Q:
                case XK_Escape:
                    running = false;
                    break;

                case XK_d:
                case XK_D:
                case XK_Right:
                    x++;
                    do_image();
                    break;

                case XK_a:
                case XK_A:
                case XK_Left:
                    x--;
                    do_image();
                    break;

                case XK_w:
                case XK_W:
                case XK_Up:
                    y--;
                    do_image();
                    break;

                case XK_s:
                case XK_S:
                case XK_Down:
                    y++;
                    do_image();
                    break;

                case XK_plus:
                case XK_equal:
                case XK_KP_Add:
                    if ((box_width / (ratio + 1)) >= 4 && (box_height / (ratio + 1)) >= 4) {
                        ratio++;
                        update_zoom_dims();
                        do_image();
                    }
                    break;

                case XK_minus:
                case XK_KP_Subtract:
                    if (ratio > 1) {
                        ratio--;
                        update_zoom_dims();
                        do_image();
                    }
                    break;
            }
        } else if (ev.type == Expose) {
            do_image();
        } else if (ev.type == MotionNotify) {
            x = ev.xmotion.x_root;
            y = ev.xmotion.y_root;

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

            do_image();
        } else if (ev.type == ButtonPress) {
            x = ev.xmotion.x_root;
            y = ev.xmotion.y_root;

            if (ev.xbutton.button == 4) {
                if ((box_width / (ratio * 2)) >= 4 && (box_height / (ratio * 2)) >= 4) {
                    ratio *= 2;
                    update_zoom_dims();
                    do_image();
                }
            } else if (ev.xbutton.button == 5) {
                if (ratio >= 2) {
                    ratio /= 2;
                    update_zoom_dims();
                    do_image();
                }
            } else if (ev.xbutton.button == 3) {
                int32_t mx = ev.xbutton.x_root - box_width / 2;
                int32_t my = ev.xbutton.y_root - box_height / 2;
                if (mx < 10) mx = 10;
                if (my < 10) my = 10;
                if (mx > dpy_width - win_width - 10) mx = dpy_width - win_width - 10;
                if (my > dpy_height - win_height - 10) my = dpy_height - win_height - 10;

                base_x = mx;
                base_y = my;
                opposite_x = dpy_width - win_width - base_x;
                opposite_y = dpy_height - win_height - base_y;

                if (opposite_x < 10) opposite_x = 10;
                if (opposite_y < 10) opposite_y = 10;
                if (opposite_x > dpy_width - win_width - 10) opposite_x = dpy_width - win_width - 10;
                if (opposite_y > dpy_height - win_height - 10) opposite_y = dpy_height - win_height - 10;

                shifted = false;
                XMoveWindow(dpy, win, base_x, base_y);
            } else {
                running = false;
            }
        }
    }

    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    free(image_data);

    return 0;
}

