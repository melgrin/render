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

// RGBA
#define BYTES_PER_PIXEL sizeof(uint32_t)

// 60 fps, microseconds
#define TARGET_ELAPSED_TIME 16667

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

        XImage* image = XCreateImage(
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

void process_keyboard_message(bool isDown, bool /*wasDown*/, Render::Button_State* state) {
    if (isDown != state->endedDown) {
        state->endedDown = isDown;
    }
    //TODO no equivalent of wasDown in X11 within the message itself, so will need to store it during the frame.  But also, downCount and upCount aren't used by Render right now, so maybe I should just get rid of them.
    //if (isDown) {
    //    state->downCount += 1;
    //} else if (wasDown) {
    //    state->upCount += 1;
    //}
}

void process_mouse_button_message(bool isDown, Render::Button_State* state) {
    if (isDown != state->endedDown) {
        state->endedDown = isDown;
        //++state->halfTransistionCount;
    }
}

#define ProcessKBMessage(KEYBOARD_BUTTON_INDEX) process_keyboard_message(isDown, wasDown, &input->keyboard.buttons[KEYBOARD_BUTTON_INDEX])

bool process_pending_messages(Display* display, Window window, Render::Input* input, int keysyms_per_keycode) {
    bool running = true;

    XEvent event = {};

    while (XPending(display)) {

        XNextEvent(display, &event);
        switch (event.type) {

            case KeyPress: // fallthrough
            case KeyRelease: {
                XKeyEvent* key_event = (XKeyEvent*) &event;
                //assert(key_event->display == display);
                //assert(key_event->window == window);
                //printf("key press: state = %u, detail = %u\n", key_event->state, key_event->keycode);
                bool isDown = (event.type == KeyPress);
                bool wasDown = false; // TODO, unused
                for (int i = 0; i < keysyms_per_keycode; ++i) {
                    KeySym key = XLookupKeysym(key_event, i);

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

                    if      (key == XK_w) { ProcessKBMessage(Render::KEYBOARD_BUTTON_UP); }
                    else if (key == XK_s) { ProcessKBMessage(Render::KEYBOARD_BUTTON_DOWN); }
                    else if (key == XK_a) { ProcessKBMessage(Render::KEYBOARD_BUTTON_LEFT); }
                    else if (key == XK_d) { ProcessKBMessage(Render::KEYBOARD_BUTTON_RIGHT); }
                    else if (key == XK_g) { ProcessKBMessage(Render::KEYBOARD_BUTTON_G); }
                    else if (key == XK_r) { ProcessKBMessage(Render::KEYBOARD_BUTTON_R); }
                    else if (key == XK_q) { ProcessKBMessage(Render::KEYBOARD_BUTTON_Q); }
                    else if (key == XK_e) { ProcessKBMessage(Render::KEYBOARD_BUTTON_E); }
                    else if (key == XK_1) { ProcessKBMessage(Render::KEYBOARD_BUTTON_1); }
                    else if (key == XK_2) { ProcessKBMessage(Render::KEYBOARD_BUTTON_2); }
                    else if (key == XK_3) { ProcessKBMessage(Render::KEYBOARD_BUTTON_3); }
                    else if (key == XK_4) { ProcessKBMessage(Render::KEYBOARD_BUTTON_4); }
                    //else if (key == VK_UP) { ProcessKBMessage(Render::KEYBOARD_BUTTON_UP); }
                    //else if (key == VK_DOWN) { ProcessKBMessage(Render::KEYBOARD_BUTTON_DOWN); }
                    //else if (key == VK_LEFT) { ProcessKBMessage(Render::KEYBOARD_BUTTON_LEFT); }
                    //else if (key == VK_RIGHT) { ProcessKBMessage(Render::KEYBOARD_BUTTON_RIGHT); }
                    else if (key == XK_Escape) {
                        running = false;
                    }
                }
            }
            break;

            // mouse button
            case ButtonPress: // fallthrough
            case ButtonRelease: {
                XButtonEvent* button_event = (XButtonEvent*) &event;
                //printf("mouse pressed:  x %4d   y %4d   state %2d   button %2d\n",
                //    button_event->x,
                //    button_event->y,
                //    button_event->state,
                //    button_event->button);

                //button_event->state is for things like shift and control

                input->mouse.cursor.position.x = button_event->x;
                input->mouse.cursor.position.y = button_event->y;

                bool isDown = event.type == ButtonPress;
                switch (button_event->button) {
                    case Button1:
                        process_mouse_button_message(isDown, &input->mouse.buttons[Render::MOUSE_BUTTON_LEFT]);
                        break;
                    case Button2:
                        process_mouse_button_message(isDown, &input->mouse.buttons[Render::MOUSE_BUTTON_MIDDLE]);
                        break;
                    case Button3:
                        process_mouse_button_message(isDown, &input->mouse.buttons[Render::MOUSE_BUTTON_RIGHT]);
                        break;
                }
            }
            break;

            // mouse pointer
            case MotionNotify: {
                XMotionEvent* motion_event = (XMotionEvent*) &event;
                //printf("x %3d  y %3d  state %#x\n", 
                //    motion_event->x,
                //    motion_event->y,
                //    motion_event->state);
                // state is which mouse button is down during the motion.  I'm just going to rely on ButtonPress instead for now.
                input->mouse.cursor.position.x = motion_event->x;
                input->mouse.cursor.position.y = motion_event->y;

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

            case ConfigureNotify: { // StructureNotifyMask
                //XConfigureEvent* configure_event = (XConfigureEvent*) &event;
                //printf("w %4d   h %4d   x %4d   y %4d\n",
                //    configure_event->width, configure_event->height,
                //    configure_event->x, configure_event->y);
                resize_DIB_section(display, window, &X11::g_bitmap);
            }
            break;

            case MapNotify: // StructureNotifyMask
            case ReparentNotify: { // StructureNotifyMask
                // no-op
            }
            break;

            default: {
                // See X.h line ~181. Not sure if there's a to_string.
                fprintf(stderr, "Unhandled event %d\n", event.type);
            }
            break;
        }
    }

    return running;
}

} // namespace X11

static u64 get_time_usec() {
    static struct timespec ts;
    if (0 == clock_gettime(CLOCK_MONOTONIC, &ts)) {
        u64 t = ts.tv_sec * 1000000;
        t += (ts.tv_nsec / 1000);
        return t;
    } else {
        return 0;
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
        400,                         // width
        400,                         // height
        0,                           // border_width
        0,                           // border
        0);                          // background

    resize_DIB_section(display, window, &X11::g_bitmap);

    GC gc = XCreateGC(display, window, 0, NULL);

    int keysyms_per_keycode;
    {
        int min_keycode, max_keycode;
        XDisplayKeycodes(display, &min_keycode, &max_keycode);
        XFree(XGetKeyboardMapping(display, min_keycode, max_keycode + 1 - min_keycode, &keysyms_per_keycode));
    }

    X11::g_wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &X11::g_wm_delete_window, 1);

    // TODO mouse
    XSelectInput(display, window,
        KeyPressMask |
        KeyReleaseMask |
        ButtonPressMask | 
        ButtonReleaseMask |
        StructureNotifyMask |
        PointerMotionMask |
        ButtonMotionMask);

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
        u64 last_frame_end_time = get_time_usec();

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


            if (!X11::process_pending_messages(display, window, newInput, keysyms_per_keycode)) {
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

            {
                u64 work_end_time = get_time_usec();
                u64 work_elapsed_time = work_end_time - last_frame_end_time;
                if (work_elapsed_time > TARGET_ELAPSED_TIME) {
                    printf("overframed: %lu\n", work_elapsed_time); // this happens quite a bit when fullscreen on 1920x1080
                } else {
                    u64 remaining = TARGET_ELAPSED_TIME - work_elapsed_time;
                    usleep(remaining);
                }
            }

            X11::display_bitmap(display, window, gc, &X11::g_bitmap);

            swap(newInput, oldInput);

            last_frame_end_time = get_time_usec();
        }

    } else {
        fprintf(stderr, "malloc failed: error %d: %s\n", errno, strerror(errno));
    }

    XCloseDisplay(display);
    return 0;
}
