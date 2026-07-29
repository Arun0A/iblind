#ifndef X11_H
#define X11_H

#include <X11/Xlib.h>
#include <X11/keysym.h>

void grab_key(Display *dpy, Window root, KeySym keysym);
void grab_key_mod(Display *dpy, Window root, KeySym keysym, unsigned int modifiers);
void set_always_on_top(Display *dpy, Window win);

#endif
