#ifndef RENDERDEV_RENDER_H
#define RENDERDEV_RENDER_H

#include "common.h"
#include "font.h"

namespace Render {

struct Offscreen_Bitmap_Buffer {
    void* memory;
    int width;
    int height;
    int pitch; // number of bytes composing a row (row-to-row distance)
};

struct Vector2 {
    f32 x;
    f32 y;
};

struct Vector2i {
    int x;
    int y;
};

struct Button_State {
    //int halfTransistionCount; // the number of ups+downs in a frame
    bool endedDown; // whether the button was down at the end of the frame
    bool changed; // whether the button state (up/down) changed from the last frame
    int downCount; // the number of times the button was pushed down during the frame
    int upCount; // the number of times the button was released during the frame
};

enum Keyboard_Button_Index {
   KEYBOARD_BUTTON_UP,
   KEYBOARD_BUTTON_DOWN,
   KEYBOARD_BUTTON_LEFT,
   KEYBOARD_BUTTON_RIGHT,

   KEYBOARD_BUTTON_1,
   KEYBOARD_BUTTON_2,
   KEYBOARD_BUTTON_3,
   KEYBOARD_BUTTON_4,

   KEYBOARD_BUTTON_G,
   KEYBOARD_BUTTON_R,
   KEYBOARD_BUTTON_Q,
   KEYBOARD_BUTTON_E,
   KEYBOARD_BUTTON_T,
   KEYBOARD_BUTTON_U,

   KEYBOARD_BUTTON_COUNT
};

struct Keyboard_Input {
    bool isConnected;
    Button_State buttons[KEYBOARD_BUTTON_COUNT];
};

enum Mouse_Button_Index {
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_RIGHT,

    MOUSE_BUTTON_COUNT
};

struct Mouse_Cursor {
    Vector2i position;
    Vector2i change;
};

struct Mouse_Wheel {
    int delta;
    Vector2i position;
    //int modifier;
};

struct Mouse_Input {
    Mouse_Cursor cursor;
    Button_State buttons[MOUSE_BUTTON_COUNT];
    Mouse_Wheel wheel;
};

struct Memory {
    bool  isInitialized;
    u64   permanentStorageSize;
    void* permanentStorage; // should be cleared to zero at startup (For Win32, VirtualAlloc does this)
    u64   transientStorageSize;
    void* transientStorage; // should be cleared to zero at startup (For Win32, VirtualAlloc does this)
};

struct Input {
    Keyboard_Input keyboard;
    Mouse_Input mouse;
};

void update(Memory* memory, Input* input, Offscreen_Bitmap_Buffer* bitmapBuffer);

} // namespace Render

#endif // RENDERDEV_RENDER_H
