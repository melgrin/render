
#ifdef RENDERDEV_ASSERT
#define assert(X) if (!(X)) { *(int*)0 = 0; }
#else
#define assert(X)
#endif

#ifdef RENDERDEV_DEBUG
namespace Platform {
    void DEBUG_printf(const char* format, ...);
    void DEBUG_display(const char* format, ...);
}
#endif

#define arrayCount(X) (sizeof(X)/sizeof((X)[0]))

#define swap(A, B) do { \
    decltype(A) temp = A; \
    A = B; \
    B = temp; \
} while (0)

#define kilobytes(X) ((X)*1024)
#define megabytes(X) (kilobytes(X)*1024)
#define gigabytes(X) (megabytes(X)*1024)
#define terabytes(X) (gigabytes(X)*1024)

#include <stdlib.h> // rand

// I think this might only be for Win32, in which case it should go in win32.cpp, which should provide a platform color value creation function.
global const int BLUE_BIT_OFFSET = 0;
global const int GREEN_BIT_OFFSET = 8;
global const int RED_BIT_OFFSET = 16;

u32 get_color(u8 red, u8 green, u8 blue) {
    u32 color = red << RED_BIT_OFFSET | green << GREEN_BIT_OFFSET | blue << BLUE_BIT_OFFSET;
    return color;
}

namespace Render {

struct Offscreen_Bitmap_Buffer {
    void* memory;
    int width;
    int height;
    int pitch; // number of bytes composing a row (row-to-row distance)
};

void draw_weird_gradient(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yOffset) {
    u8* row = (u8*) bitmapBuffer->memory;
    for (int y = 0; y < bitmapBuffer->height; ++y) {
        u32* pixel = (u32*) row;
        for (int x = 0; x < bitmapBuffer->width; ++x) {

            u8 blue = (u8)(x - xOffset);
            u8 green = (u8)(y - yOffset);
            //*pixel = ((green << 0) | (blue << BLUE_BIT_OFFSET));
            //*pixel = ((green << RED_BIT_OFFSET) | (blue << BLUE_BIT_OFFSET));
            *pixel = ((green << GREEN_BIT_OFFSET) | (blue << BLUE_BIT_OFFSET));
            ++pixel;
        }

        row += bitmapBuffer->pitch;
    }
}

void draw_test_color_bands(Offscreen_Bitmap_Buffer* bitmapBuffer/*, int xOffset, int yOffset*/) {
    u8* row = (u8*) bitmapBuffer->memory;

    u8 intensity = 0;
    int bitOffset = 0;
    for (int y = 0; y < bitmapBuffer->height; ++y) {
        u32* pixel = (u32*) row;
        for (int x = 0; x < bitmapBuffer->width; ++x) {
            *pixel = intensity << bitOffset;
            ++pixel;
        }

        row += bitmapBuffer->pitch;

        ++intensity;
        if (intensity == 0) {
            bitOffset += 8;
            if (bitOffset > 16) {
                bitOffset = 0;
            }
        }
    }
}

void fill_color(Offscreen_Bitmap_Buffer* bitmapBuffer, u32 color) {
    u32* pixel = (u32*) bitmapBuffer->memory;
    int n = bitmapBuffer->height * bitmapBuffer->width;
    for (int i = 0; i < n; ++i) {
        *pixel = color;
        ++pixel;
    }
}

void fill_pixel(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yOffset, u32 color) {
    u8* row = (u8*) bitmapBuffer->memory;
    row += bitmapBuffer->pitch * yOffset;
    u32* pixel = (u32*) row;
    pixel += xOffset;
    *pixel = color;
}

void draw_horizontal_line_segment(Offscreen_Bitmap_Buffer* bitmapBuffer, int yOffset, int xStart, int xEnd, u32 color) {

    if (yOffset >= bitmapBuffer->height) return;
    if (yOffset < 0) return;

    assert(xStart < xEnd);

    if (xStart < 0) xStart = 0;
    if (xEnd >= bitmapBuffer->width) xEnd = bitmapBuffer->width;

    u8* row = (u8*) bitmapBuffer->memory;
    row += bitmapBuffer->pitch * yOffset;
    u32* pixel = (u32*) row;
    pixel += xStart;
    for (int x = xStart; x < xEnd; ++x) {
        *pixel = color;
        ++pixel;
    }
}

void draw_vertical_line_segment(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yStart, int yEnd, u32 color) {
    
    // xOffset starts at 0, so == width is invalid
    if (xOffset >= bitmapBuffer->width) return;
    if (xOffset < 0) return;

    assert(yStart < yEnd);

    if (yStart < 0) yStart = 0;
    if (yEnd >= bitmapBuffer->height) yEnd = bitmapBuffer->height;

    u8* row = (u8*) bitmapBuffer->memory;
    row += bitmapBuffer->pitch * yStart;

    for (int y = yStart; y < yEnd; ++y) {
        u32* pixel = (u32*) row;
        pixel += xOffset;
        *pixel = color;
        row += bitmapBuffer->pitch;
    }
}

void draw_crosshair(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yOffset, int hairLength) {

    local_persist u32 COLOR_HORIZONTAL = get_color(255, 0, 0);
    local_persist u32 COLOR_VERTICAL = get_color(0, 255, 0);
    //local_persist u32 COLOR_BACKGROUND = get_color(0, 0, 0);

    //Platform::DEBUG_printf("drawing at {%4i, %4i}\n");

    //fill_color(bitmapBuffer, COLOR_BACKGROUND);
    draw_horizontal_line_segment(bitmapBuffer, yOffset, xOffset - hairLength, xOffset + hairLength, COLOR_HORIZONTAL);
    draw_vertical_line_segment(bitmapBuffer, xOffset, yOffset - hairLength, yOffset + hairLength, COLOR_VERTICAL);
}

void draw_grid(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yOffset) {

    local_persist u32 GRID_SPACING = 40;
    local_persist u32 COLOR_FG = get_color(0, 255, 0);
    //local_persist u32 COLOR_BG = get_color(0, 0, 0);

    //fill_color(bitmapBuffer, COLOR_BG);

    for (int y = yOffset; y < bitmapBuffer->height; y += GRID_SPACING) {
        draw_horizontal_line_segment(bitmapBuffer, y, 0, bitmapBuffer->width-1, COLOR_FG);
    }
    for (int y = yOffset; y >= 0; y -= GRID_SPACING) {
        draw_horizontal_line_segment(bitmapBuffer, y, 0, bitmapBuffer->width-1, COLOR_FG);
    }

    for (int x = xOffset; x < bitmapBuffer->width; x += GRID_SPACING) {
        draw_vertical_line_segment(bitmapBuffer, x, 0, bitmapBuffer->height-1, COLOR_FG);
    }
    for (int x = xOffset; x > 0; x -= GRID_SPACING) {
        draw_vertical_line_segment(bitmapBuffer, x, 0, bitmapBuffer->height-1, COLOR_FG);
    }
}

struct Rectangle {
    int x;
    int y;
    int w;
    int h;
    u32 border_color;
    //u32 fill_color;
};

void draw_rect_old(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yOffset) {
    //local_persist u32 COLOR_BG = get_color(60, 60, 60);
    //fill_color(bitmapBuffer, COLOR_BG);

    local_persist u16 RECT_SIZE = 40;
    local_persist u32 COLOR_FG = get_color(120, 200, 255);
    draw_horizontal_line_segment(bitmapBuffer, yOffset            , xOffset, xOffset + RECT_SIZE, COLOR_FG); // top
    draw_horizontal_line_segment(bitmapBuffer, yOffset + RECT_SIZE, xOffset, xOffset + RECT_SIZE, COLOR_FG); // bottom
    draw_vertical_line_segment  (bitmapBuffer, xOffset            , yOffset, yOffset + RECT_SIZE, COLOR_FG); // left
    draw_vertical_line_segment  (bitmapBuffer, xOffset + RECT_SIZE, yOffset, yOffset + RECT_SIZE, COLOR_FG); // right

}

void draw_rect(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yOffset, Rectangle* rect) {
    int x0 = rect->x + xOffset;
    int x1 = rect->x + xOffset + rect->w;
    int y0 = rect->y + yOffset;
    int y1 = rect->y + yOffset + rect->h;
    draw_horizontal_line_segment(bitmapBuffer, y0, x0, x1, rect->border_color); // top
    draw_horizontal_line_segment(bitmapBuffer, y1, x0, x1, rect->border_color); // bottom
    draw_vertical_line_segment  (bitmapBuffer, x0, y0, y1, rect->border_color); // left
    draw_vertical_line_segment  (bitmapBuffer, x1, y0, y1, rect->border_color); // right
}

struct Vector2 {
    int x;
    int y;
};

struct Button_State {
    int halfTransistionCount;
    bool endedDown;
    bool changed;
};

enum Keyboard_Button_Index {
   KEYBOARD_BUTTON_UP,
   KEYBOARD_BUTTON_DOWN,
   KEYBOARD_BUTTON_LEFT,
   KEYBOARD_BUTTON_RIGHT,

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
    Vector2 position;
    Vector2 change;
};

struct Mouse_Input {
    Mouse_Cursor cursor;
    Button_State buttons[MOUSE_BUTTON_COUNT];
};

struct State {
    int xOffset;
    int yOffset;
    bool mouseDragging;
    Rectangle* rectangles;
    int rectanglesCount;
};

struct Memory {
    bool isInitialized;
    u64 permanentStorageSize;
    void* permanentStorage; // should be cleared to zero at startup (For Win32, VirtualAlloc does this)
    u64 transientStorageSize;
    void* transientStorage; // should be cleared to zero at startup (For Win32, VirtualAlloc does this)
};

struct Crosshair {
    int x;
    int y;
};

struct Input {
    Keyboard_Input keyboard;
    Mouse_Input mouse;
};

void update(Memory* memory, Input* input, Offscreen_Bitmap_Buffer* bitmapBuffer) {

    assert(sizeof(State) <= memory->permanentStorageSize);
    State* state = (State*) memory->permanentStorage;
    local_persist Crosshair* next_ch = 0;
    local_persist Crosshair* first_ch = 0;
    local_persist u32 num_ch = 0;
    if (!memory->isInitialized) {

        u8* p = (u8*) memory->permanentStorage;
        p += sizeof(State);

        u32 rect_color = get_color(222, 126, 45);

        state->rectangles = (Rectangle*) p;
        state->rectanglesCount = 20;
        for (int i = 0; i < state->rectanglesCount; ++i) {
            Rectangle* rect = ((Rectangle*) p) + i;

            rect->x = rand() % 1000;
            rect->y = rand() % 500;
            rect->w = rand() % 100 + 1;
            rect->h = rand() % 100 + 1;
            rect->border_color = rect_color;
        }
        p += sizeof(Rectangle) * state->rectanglesCount;

        next_ch = (Crosshair*) p;
        first_ch = next_ch;

        memory->isInitialized = true;
    }

#if 0
    {
        Platform::DEBUG_display("mouse pressed [%d -> %d]    drag start [%d %d]    drag end [%d %d]    offset [%d %d]",
            state->wasMousePressed, mouse->isButtonPressed,
            state->mouseDragStart.x, state->mouseDragStart.y,
            state->mouseDragEnd.x, state->mouseDragEnd.y,
            state->xOffset, state->yOffset);
    }
#endif


    Mouse_Cursor* cursor = &input->mouse.cursor;

    //for (int i = 0; i < arrayCount(input->mouse.buttons); ++i) {
    {
        int i = MOUSE_BUTTON_LEFT; // XXX harcode for now. might break with multiple buttons being able to control drag start/stop

        Button_State* button = &input->mouse.buttons[i];

        if (button->changed && button->endedDown) {
            next_ch->x = cursor->position.x;
            next_ch->y = cursor->position.y;
            ++num_ch;
            ++next_ch;
        }

        if (button->changed) {
            state->mouseDragging = button->endedDown;
        }
    }

    if (state->mouseDragging) {
        state->xOffset += cursor->change.x;
        state->yOffset += cursor->change.y;
    }

#if 0
    {
        if (keyboard->buttons[BUTTON_UP].endedDown) {
            state->yOffset -= 1;
        } else if (keyboard->buttons[BUTTON_DOWN].endedDown) {
            state->yOffset += 1;
        } else if (keyboard->buttons[BUTTON_LEFT].endedDown) {
            state->xOffset -= 1;
        } else if (keyboard->buttons[BUTTON_RIGHT].endedDown) {
            state->xOffset += 1;
        }
    }
#endif

    //draw_weird_gradient(bitmapBuffer, state->xOffset, state->yOffset);
    //draw_test_color_bands(bitmapBuffer);
    //draw_crosshair(bitmapBuffer, state->xOffset, state->yOffset);
    //draw_grid(bitmapBuffer, state->xOffset, state->yOffset);

    local_persist u32 COLOR_BG = get_color(60, 60, 60);
    fill_color(bitmapBuffer, COLOR_BG);

    Crosshair* ch = first_ch;
    for (u32 i = 0; i < num_ch; ++i) {
        draw_crosshair(bitmapBuffer, ch->x, ch->y, 20);
        ++ch;
    }

    for (int i = 0; i < state->rectanglesCount; ++i) {
        draw_rect(bitmapBuffer, state->xOffset, state->yOffset, &state->rectangles[i]);
    }

    // TODO draw arbitrary points, then preserve their position while panning

}

} // namespace Render

