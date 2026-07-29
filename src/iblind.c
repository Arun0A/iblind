/*
    iblind: A customizable screen magnifier for X11
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "globals.h"
#include "x11.h"
#include "magnifier.h"

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
                ratio = atof(argv[++i]);
            } else {
                fprintf(stderr, "Error: %s requires an argument\n", argv[i]);
                return -1;
            }
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--color") == 0) {
            if (i + 1 < argc) {
                border_color = strtoul(argv[++i], NULL, 16);
            } else {
                fprintf(stderr, "Error: %s requires an argument\n", argv[i]);
                return -1;
            }
        } else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--no-border") == 0) {
            padding_val = 0;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--persist") == 0) {
            persist_mode = true;
        } else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--grab") == 0) {
            persist_mode = false;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-?") == 0) {
            printf("Usage: iblind [options]\n");
            printf("Options:\n");
            printf("  -w, --width <val>   Width of the magnifier box (default: 800)\n");
            printf("  -h, --height <val>  Height of the magnifier box (default: 450)\n");
            printf("  -x, --xpos <val>    Initial X position of the window\n");
            printf("  -y, --ypos <val>    Initial Y position of the window\n");
            printf("  -z, --zoom <val>    Default zoom level (default: 2.0)\n");
            printf("  -c, --color <hex>   Border color in hex (default: 9C9C9C)\n");
            printf("  -b, --no-border     Disable borders completely\n");
            printf("  -p, --persist       Persist through clicks (default: enabled)\n");
            printf("  -g, --grab          Enable grab mode (modal, exit on click)\n");
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
    if (ratio < 1.0f) ratio = 1.0f;

    int max_zoom_w = box_width / 4;
    int max_zoom_h = box_height / 4;
    int max_zoom = max_zoom_w < max_zoom_h ? max_zoom_w : max_zoom_h;
    if (max_zoom < 1) max_zoom = 1;
    if (ratio > max_zoom) ratio = max_zoom;

    win_width = box_width + padding_val * 2;
    win_height = box_height + padding_val * 2;

    update_zoom_dims();

    image_data = malloc(box_width * box_height * 4);
    if (!image_data) {
        fprintf(stderr, "Error: failed to allocate memory for image data\n");
        free(map_src_word_x);
        free(map_src_word_y);
        return -1;
    }

    if ((dpy = XOpenDisplay(NULL)) == NULL) {
        fprintf(stderr, "Error: can't open display :(\n");
        free(image_data);
        free(map_src_word_x);
        free(map_src_word_y);
        return -1;
    }

    screen = DefaultScreen(dpy);
    dpy_width = DisplayWidth(dpy, screen);
    dpy_height = DisplayHeight(dpy, screen);
    visual = DefaultVisual(dpy, screen);
    depth = DefaultDepth(dpy, screen);
    root = DefaultRootWindow(dpy);

    base_x = has_init_x ? init_x : (dpy_width - (win_width + margin_val));
    base_y = has_init_y ? init_y : (dpy_height - (win_height + margin_val));

    if (base_x < margin_val) base_x = margin_val;
    if (base_y < margin_val) base_y = margin_val;
    if (base_x > dpy_width - win_width - margin_val) base_x = dpy_width - win_width - margin_val;
    if (base_y > dpy_height - win_height - margin_val) base_y = dpy_height - win_height - margin_val;

    opposite_x = dpy_width - win_width - base_x;
    opposite_y = dpy_height - win_height - base_y;

    if (opposite_x < margin_val) opposite_x = margin_val;
    if (opposite_y < margin_val) opposite_y = margin_val;
    if (opposite_x > dpy_width - win_width - margin_val) opposite_x = dpy_width - win_width - margin_val;
    if (opposite_y > dpy_height - win_height - margin_val) opposite_y = dpy_height - win_height - margin_val;

    win = XCreateSimpleWindow(
        dpy, root,
        base_x, base_y,
        win_width, win_height,
        0, 0, border_color
    );

    gc = XCreateGC(dpy, win, 0, NULL);

    XSizeHints sh;
    sh.min_width  = sh.max_width  = win_width;
    sh.min_height = sh.max_height = win_height;
    sh.flags = PMinSize | PMaxSize;
    XSetWMNormalHints(dpy, win, &sh);

    XStoreName(dpy, win, "iblind");
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

    if (persist_mode) {
        set_always_on_top(dpy, win);
    } else {
        XGrabPointer(
            dpy, root, true,
            PointerMotionMask | ButtonPressMask,
            GrabModeAsync, GrabModeAsync, None,
            c, CurrentTime
        );
    }

    if (persist_mode) {
        // In persist mode, globally grab Alt + keys
        grab_key_mod(dpy, root, XK_q, Mod1Mask);
        grab_key_mod(dpy, root, XK_Q, Mod1Mask);
        grab_key_mod(dpy, root, XK_Escape, Mod1Mask);
        grab_key_mod(dpy, root, XK_plus, Mod1Mask);
        grab_key_mod(dpy, root, XK_minus, Mod1Mask);
        grab_key_mod(dpy, root, XK_equal, Mod1Mask);
        grab_key_mod(dpy, root, XK_KP_Add, Mod1Mask);
        grab_key_mod(dpy, root, XK_KP_Subtract, Mod1Mask);
    } else {
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
    }

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

        if (!found) {
            usleep(10000); // yield CPU for 10ms
            n++;
            if (n > 3) { // ~30ms periodic refresh
                n = 0;
                do_image();
            }
            continue;
        }

        n = 0; // reset refresh timer

        if (ev.type == KeyPress) {
            KeySym keysym = XLookupKeysym(&ev.xkey, 0);
            unsigned int state = ev.xkey.state;
            state &= ~(LockMask | Mod2Mask);
            bool alt_pressed = (state & Mod1Mask) != 0;

            if (persist_mode && !alt_pressed) {
                continue;
            }

            switch (keysym) {
                case XK_q:
                case XK_Q:
                case XK_Escape:
                    running = false;
                    break;

                case XK_d:
                case XK_D:
                case XK_Right:
                    if (!persist_mode) {
                        x++;
                        do_image();
                    }
                    break;

                case XK_a:
                case XK_A:
                case XK_Left:
                    if (!persist_mode) {
                        x--;
                        do_image();
                    }
                    break;

                case XK_w:
                case XK_W:
                case XK_Up:
                    if (!persist_mode) {
                        y--;
                        do_image();
                    }
                    break;

                case XK_s:
                case XK_S:
                case XK_Down:
                    if (!persist_mode) {
                        y++;
                        do_image();
                    }
                    break;

                case XK_plus:
                case XK_equal:
                case XK_KP_Add:
                    if ((box_width / (ratio + 0.25f)) >= 4 && (box_height / (ratio + 0.25f)) >= 4) {
                        ratio += 0.25f;
                        update_zoom_dims();
                        do_image();
                    }
                    break;

                case XK_minus:
                case XK_KP_Subtract:
                    if (ratio > 1.0f) {
                        ratio -= 0.25f;
                        if (ratio < 1.0f) ratio = 1.0f;
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
                if ((box_width / (ratio * 1.5f)) >= 4 && (box_height / (ratio * 1.5f)) >= 4) {
                    ratio *= 1.5f;
                    update_zoom_dims();
                    do_image();
                }
            } else if (ev.xbutton.button == 5) {
                if (ratio >= 1.5f) {
                    ratio /= 1.5f;
                    update_zoom_dims();
                    do_image();
                }
            } else if (ev.xbutton.button == 3) {
                int32_t mx = ev.xbutton.x_root - box_width / 2;
                int32_t my = ev.xbutton.y_root - box_height / 2;
                if (mx < margin_val) mx = margin_val;
                if (my < margin_val) my = margin_val;
                if (mx > dpy_width - win_width - margin_val) mx = dpy_width - win_width - margin_val;
                if (my > dpy_height - win_height - margin_val) my = dpy_height - win_height - margin_val;

                base_x = mx;
                base_y = my;
                opposite_x = dpy_width - win_width - base_x;
                opposite_y = dpy_height - win_height - base_y;

                if (opposite_x < margin_val) opposite_x = margin_val;
                if (opposite_y < margin_val) opposite_y = margin_val;
                if (opposite_x > dpy_width - win_width - margin_val) opposite_x = dpy_width - win_width - margin_val;
                if (opposite_y > dpy_height - win_height - margin_val) opposite_y = dpy_height - win_height - margin_val;

                shifted = false;
                XMoveWindow(dpy, win, base_x, base_y);
            } else {
                if (!persist_mode) {
                    running = false;
                }
            }
        }
    }

    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    free(image_data);
    free(map_src_word_x);
    free(map_src_word_y);

    return 0;
}
