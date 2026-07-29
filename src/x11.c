#include "x11.h"
#include <string.h>
#include <stdbool.h>

void grab_key(Display *dpy, Window root, KeySym keysym) {
    KeyCode code = XKeysymToKeycode(dpy, keysym);
    if (code) {
        XGrabKey(dpy, code, 0, root, true, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, code, ShiftMask, root, true, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, code, LockMask, root, true, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, code, ShiftMask | LockMask, root, true, GrabModeAsync, GrabModeAsync);
    }
}

void grab_key_mod(Display *dpy, Window root, KeySym keysym, unsigned int modifiers) {
    KeyCode code = XKeysymToKeycode(dpy, keysym);
    if (code) {
        XGrabKey(dpy, code, modifiers, root, true, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, code, modifiers | LockMask, root, true, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, code, modifiers | Mod2Mask, root, true, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy, code, modifiers | Mod2Mask | LockMask, root, true, GrabModeAsync, GrabModeAsync);
    }
}

void set_always_on_top(Display *dpy, Window win) {
    Atom wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
    Atom wm_above = XInternAtom(dpy, "_NET_WM_STATE_ABOVE", False);

    XClientMessageEvent xev;
    memset(&xev, 0, sizeof(xev));
    xev.type = ClientMessage;
    xev.window = win;
    xev.message_type = wm_state;
    xev.format = 32;
    xev.data.l[0] = 1; // _NET_WM_STATE_ADD
    xev.data.l[1] = (long)wm_above;
    xev.data.l[2] = 0;
    xev.data.l[3] = 0;
    xev.data.l[4] = 0;

    XSendEvent(dpy, DefaultRootWindow(dpy), False,
               SubstructureRedirectMask | SubstructureNotifyMask,
               (XEvent *)&xev);
}
