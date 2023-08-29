// https://www.x.org/docs/X11/xlib.pdf
// https://tronche.com/gui/x/xlib/

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h> // see X11/keysymdef.h line ~600

#define GRID_WIDTH_PX 400
#define GRID_HEIGHT_PX 400
typedef uint32_t RGBA32;

static void fill_rgba32(RGBA32 grid[GRID_WIDTH_PX * GRID_HEIGHT_PX], RGBA32 value) {
    for (size_t i = 0; i < GRID_WIDTH_PX * GRID_HEIGHT_PX; ++i) {
        grid[i] = value;
    }
}

int main() {

    Display* display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "XOpenDisplay failed\n");
        exit(1);
    }

    Window window = XCreateSimpleWindow(
        display,
        XDefaultRootWindow(display), // parent
        0,                           // x
        0,                           // y
        GRID_WIDTH_PX,               // width
        GRID_HEIGHT_PX,              // height
        0,                           // border_width
        0,                           // border
        0);                          // background

    XWindowAttributes wa = {0};
    XGetWindowAttributes(display, window, &wa);

    GC gc = XCreateGC(display, window, 0, NULL);

    RGBA32 grid_px[GRID_WIDTH_PX * GRID_HEIGHT_PX] = {0};
    for (size_t i = 0; i < GRID_WIDTH_PX * GRID_HEIGHT_PX; ++i) {
        grid_px[i] = 0xffffff;
    }

    XImage* image = XCreateImage(
        display,
        wa.visual,
        wa.depth,
        ZPixmap,                         // format
        0,                               // offset
        (char*) grid_px,                 // data
        GRID_WIDTH_PX,
        GRID_HEIGHT_PX,
        sizeof(grid_px[0]) * 8,          // bitmap_pad
        GRID_WIDTH_PX * sizeof(RGBA32)); // bytes_per_line

    int min_keycode, max_keycode;
    XDisplayKeycodes(display, &min_keycode, &max_keycode);
    int keysyms_per_keycode;
    XFree(XGetKeyboardMapping(display, min_keycode, max_keycode + 1 - min_keycode, &keysyms_per_keycode));
    //printf("keysyms_per_keycode = %d\n", keysyms_per_keycode);

    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    XSelectInput(display, window, KeyPressMask);

    XMapWindow(display, window);

    XSync(display, False);

    int running = 1;
    while (running) {
        XEvent event = {0};
        XNextEvent(display, &event);
        switch (event.type) {

            case KeyPress: {
                XKeyEvent* key_event = (XKeyEvent*) &event;
                assert(key_event->display == display);
                assert(key_event->window == window);
                //printf("key press: state = %u, detail = %u\n", key_event->state, key_event->keycode);
                for (int i = 0; i < keysyms_per_keycode; ++i) {
                    KeySym ks = XLookupKeysym(key_event, i);

                    char c = '?';
                    if (isprint(ks)) {
                        c = (char) ks;
                    } else {
                        int ksb = 0xff & ks;
                        if (isprint(ksb)) {
                            c = (char) ksb;
                        }
                    }
                    printf("%d: kc %u -> ks %lu (%c)\n", i, key_event->keycode, ks, c);

                    switch (ks) {
                        case XK_q:
                        case XK_Escape:
                            running = 0;
                            break;
                        case XK_r:
                            fill_rgba32(grid_px, 0xff0000);
                            XPutImage(display, window, gc, image, 0, 0, 0, 0, GRID_WIDTH_PX, GRID_HEIGHT_PX);
                            break;
                        case XK_g:
                            fill_rgba32(grid_px, 0x00ff00);
                            XPutImage(display, window, gc, image, 0, 0, 0, 0, GRID_WIDTH_PX, GRID_HEIGHT_PX);
                            break;
                        case XK_b:
                            fill_rgba32(grid_px, 0x0000ff);
                            XPutImage(display, window, gc, image, 0, 0, 0, 0, GRID_WIDTH_PX, GRID_HEIGHT_PX);
                            break;
                        default:
                            break;
                    }
                }
            }
            break;

            case ClientMessage: {
                if ((Atom) event.xclient.data.l[0] == wm_delete_window) {
                    running = 0;
                }
                //XClientMessageEvent* cme = (XClientMessageEvent*) &event;
                //if (cme->message_type == wm_delete_window) {
                //    running = 0;
                //}
            }
            break;
            default:
                printf("unhandled event %d\n", event.type);
                break;
        }
    }
    XCloseDisplay(display);
    return 0;
}
