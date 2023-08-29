// https://www.x.org/docs/X11/xlib.pdf
// https://tronche.com/gui/x/xlib/

#include "common.h"
#include "render.cpp"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h> // see X11/keysymdef.h line ~600

#define GRID_WIDTH_PX 400
#define GRID_HEIGHT_PX 400

typedef uint32_t RGBA32;
#define BYTES_PER_PIXEL sizeof(RGBA32)

//static void fill_rgba32(RGBA32 grid[GRID_WIDTH_PX * GRID_HEIGHT_PX], RGBA32 value) {
//    for (size_t i = 0; i < GRID_WIDTH_PX * GRID_HEIGHT_PX; ++i) {
//        grid[i] = value;
//    }
//}

namespace X11 {

struct Offscreen_Bitmap_Buffer {
    XImage* info;
    void* memory;
    int width;
    int height;
    int pitch; // number of bytes composing a row (row-to-row distance)
};

static bool g_running;
static Atom g_wm_delete_window;
static Offscreen_Bitmap_Buffer g_bitmap;

void resize_DIB_section(Display* display, Window window, Offscreen_Bitmap_Buffer* bitmap) {
    XWindowAttributes wa = {};
    XGetWindowAttributes(display, window, &wa);

    if (wa.width != bitmap->width || wa.height != bitmap->height) {

        if (bitmap->memory) {
            free(bitmap->memory);
        }

        int nbytes = wa.width * wa.height * BYTES_PER_PIXEL;
        bitmap->memory = malloc(nbytes);
        if (!bitmap->memory) {
            fprintf(stderr, "Failed to allocate new bitmap memory. Wanted %d bytes. Error %d (%s)\n", nbytes, errno, strerror(errno));
            exit(1);
        }
        memset(bitmap->memory, 0, nbytes);

        bitmap->width  = wa.width;
        bitmap->height = wa.height;
        bitmap->pitch  = wa.width * BYTES_PER_PIXEL;

        if (bitmap->info) {
            XFree(bitmap->info);
        }

        XImage* image = XCreateImage( // TODO resize upon window resize (not sure if I can just XPutImage with a new size. probably not.)
            display,
            wa.visual,
            wa.depth,
            ZPixmap,                         // format
            0,                               // offset
            (char*) bitmap->memory,          // data
            wa.width,                        // width
            wa.height,                       // height
            BYTES_PER_PIXEL * 8,             // bitmap_pad
            bitmap->pitch);                  // bytes_per_line
        assert(image != NULL); // XCreateImage documentation doesn't say if it can return NULL, but it does. For example, if bitmap_map is not 8, 16, or 32, it will return NULL.

        bitmap->info = image;
    }

}

void display_bitmap(Display* display, Window window, GC gc, Offscreen_Bitmap_Buffer* bitmap) {
    XPutImage(display, window, gc, bitmap->info, 0, 0, 0, 0, bitmap->width, bitmap->height);
}

bool process_pending_messages(Display* display, Render::Input* /*TODO input*/, int keysyms_per_keycode) {
    bool running = true;

    XEvent event = {};

    while (XPending(display)) {

        printf("Message for you sir!\n");

        XNextEvent(display, &event);
        switch (event.type) {

            case KeyPress: {
                XKeyEvent* key_event = (XKeyEvent*) &event;
                //assert(key_event->display == display);
                //assert(key_event->window == window);
                //printf("key press: state = %u, detail = %u\n", key_event->state, key_event->keycode);
                for (int i = 0; i < keysyms_per_keycode; ++i) {
                    KeySym ks = XLookupKeysym(key_event, i);

                    //char c = '?';
                    //if (isprint(ks)) {
                    //    c = (char) ks;
                    //} else {
                    //    int ksb = 0xff & ks;
                    //    if (isprint(ksb)) {
                    //        c = (char) ksb;
                    //    }
                    //}
                    //printf("%d: kc %u -> ks %lu (%c)\n", i, key_event->keycode, ks, c);

                    switch (ks) {
                        //case XK_q:
                        case XK_Escape:
                            running = false;
                            break;
                        case XK_r:
                            //fill_rgba32(grid_px, 0xff0000);
                            //XPutImage(display, window, gc, image, 0, 0, 0, 0, GRID_WIDTH_PX, GRID_HEIGHT_PX);
                            break;
                        case XK_g:
                            //fill_rgba32(grid_px, 0x00ff00);
                            //XPutImage(display, window, gc, image, 0, 0, 0, 0, GRID_WIDTH_PX, GRID_HEIGHT_PX);
                            break;
                        case XK_b:
                            //fill_rgba32(grid_px, 0x0000ff);
                            //XPutImage(display, window, gc, image, 0, 0, 0, 0, GRID_WIDTH_PX, GRID_HEIGHT_PX);
                            break;
                        default:
                            break;
                    }
                }
            }
            break;

            case ClientMessage: {
                if ((Atom) event.xclient.data.l[0] == X11::g_wm_delete_window) {
                    running = 0;
                }
                //XClientMessageEvent* cme = (XClientMessageEvent*) &event;
                //if (cme->message_type == wm_delete_window) {
                //    running = 0;
                //}
            }
            break;

            default: {
                fprintf(stderr, "Unhandled event %d\n", event.type);
            }
            break;
        }
    }

    return running;
}

} // namespace X11

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

    resize_DIB_section(display, window, &X11::g_bitmap);

    GC gc = XCreateGC(display, window, 0, NULL);

    int min_keycode, max_keycode;
    XDisplayKeycodes(display, &min_keycode, &max_keycode);
    int keysyms_per_keycode;
    XFree(XGetKeyboardMapping(display, min_keycode, max_keycode + 1 - min_keycode, &keysyms_per_keycode));

    X11::g_wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &X11::g_wm_delete_window, 1);

    XSelectInput(display, window, KeyPressMask); // TODO mouse

    XMapWindow(display, window);

    XSync(display, False);

    Render::Memory memory = {};
    memory.permanentStorageSize = megabytes(64);
    memory.transientStorageSize = megabytes(128);
    u64 totalSize = memory.permanentStorageSize + memory.transientStorageSize;
    memory.permanentStorage = malloc(totalSize);
    if (memory.permanentStorage) {
        memory.transientStorage = (u8*) memory.permanentStorage + memory.permanentStorageSize;
        memset(memory.permanentStorage, 0, totalSize);

        Render::Input input[2] = {};
        Render::Input* newInput = &input[0];
        Render::Input* oldInput = &input[1];

        X11::g_running = true;

        while (X11::g_running) {

            Render::Keyboard_Input* oldKeyboardInput = &oldInput->keyboard;
            Render::Keyboard_Input* newKeyboardInput = &newInput->keyboard;
            *newKeyboardInput = {};
            for (size_t i = 0; i < arrayCount(newKeyboardInput->buttons); ++i) {
                newKeyboardInput->buttons[i].endedDown = 
                    oldKeyboardInput->buttons[i].endedDown;
            }

            Render::Mouse_Input* oldMouseInput = &oldInput->mouse;
            Render::Mouse_Input* newMouseInput = &newInput->mouse;
            *newMouseInput = {};
            for (size_t i = 0; i < arrayCount(newMouseInput->buttons); ++i) {
                newMouseInput->buttons[i].endedDown = 
                    oldMouseInput->buttons[i].endedDown;
            }
            newMouseInput->cursor = oldMouseInput->cursor;


            if (!X11::process_pending_messages(display, newInput, keysyms_per_keycode)) {
                X11::g_running = false;
            }

            for (size_t i = 0; i < arrayCount(newKeyboardInput->buttons); ++i) {
                newKeyboardInput->buttons[i].changed = 
                    newKeyboardInput->buttons[i].endedDown != 
                    oldKeyboardInput->buttons[i].endedDown;
            }

            for (size_t i = 0; i < arrayCount(newMouseInput->buttons); ++i) {
                newMouseInput->buttons[i].changed = 
                    newMouseInput->buttons[i].endedDown != 
                    oldMouseInput->buttons[i].endedDown;
            }

            newMouseInput->cursor.change.x =
                newMouseInput->cursor.position.x -
                oldMouseInput->cursor.position.x;

            newMouseInput->cursor.change.y =
                newMouseInput->cursor.position.y - 
                oldMouseInput->cursor.position.y;

            Render::Offscreen_Bitmap_Buffer bitmapBuffer = {};
            bitmapBuffer.memory = X11::g_bitmap.memory;
            bitmapBuffer.width  = X11::g_bitmap.width;
            bitmapBuffer.height = X11::g_bitmap.height;
            bitmapBuffer.pitch  = X11::g_bitmap.pitch;

            Render::update(&memory, newInput, &bitmapBuffer);

            //TODO enforce framerate (aka sleep for remainder of remaining frame time)

            X11::display_bitmap(display, window, gc, &X11::g_bitmap);

            swap(newInput, oldInput);

        }

    } else {
        // TODO error malloc failed
    }

    XCloseDisplay(display);
    return 0;
}
