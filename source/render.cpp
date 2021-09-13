
#ifdef RENDERDEV_ASSERT
#define assert(X) if (!(X)) { *(int*)0 = 0; }
#else
#define assert(X)
#endif

#ifdef RENDERDEV_DEBUG_PRINT
namespace Platform {
    void DEBUG_printf(const char* format, ...);
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

void draw_horizontal_line_segment(Offscreen_Bitmap_Buffer* bitmapBuffer, int yOffset, int xStart, int xEnd, u32 color) {

    // The available memory is only as large as the window.  The mouse position should never be captured outside of the window, but the line position could be left over from previous, and the window was shrunk.  In that case, can't draw the line because it's outside the window and therefore out of bounds of the allocated bitmap buffer.
    if (yOffset >= bitmapBuffer->height) {
        return;
    }

    assert(xStart < xEnd);

    if (xStart < 0) xStart = 0;
    if (xEnd >= bitmapBuffer->width) xEnd = bitmapBuffer->width;

    u8* row = (u8*) bitmapBuffer->memory;
    row += yOffset * bitmapBuffer->pitch;
    u32* pixel = (u32*) row;
    pixel += xStart;
    for (int x = xStart; x < xEnd; ++x) {
        *pixel = color;
        ++pixel;
    }
}

void draw_vertical_line_segment(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yStart, int yEnd, u32 color) {
    
    // See note in game_draw_horizontal_line_segment.  Would exceed bitmap buffer size.
    if (xOffset >= bitmapBuffer->width) { // xOffset starts at 0, so == width is invalid
        return;
    }

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

void draw_crosshair(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yOffset) {

    local_persist u32 COLOR_HORIZONTAL = get_color(255, 0, 0);
    local_persist u32 COLOR_VERTICAL = get_color(0, 255, 0);
    local_persist u32 COLOR_BACKGROUND = get_color(0, 0, 0);

    //Platform::DEBUG_printf("drawing at {%4i, %4i}\n");

    fill_color(bitmapBuffer, COLOR_BACKGROUND);
    //draw_horizontal_line_segment(bitmapBuffer, yOffset, 0, bitmapBuffer->width-1, COLOR_HORIZONTAL);
    //draw_vertical_line_segment(bitmapBuffer, xOffset, 0, bitmapBuffer->height-1, COLOR_VERTICAL);
    draw_horizontal_line_segment(bitmapBuffer, yOffset, xOffset - 100, xOffset + 100, COLOR_HORIZONTAL);
    draw_vertical_line_segment(bitmapBuffer, xOffset, yOffset - 100, yOffset + 100, COLOR_VERTICAL);
}

void draw_grid(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yOffset) {

    local_persist u32 GRID_SPACING = 40;
    local_persist u32 COLOR_FG = get_color(0, 255, 0);
    local_persist u32 COLOR_BG = get_color(0, 0, 0);

    fill_color(bitmapBuffer, COLOR_BG);

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

void draw_rect(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yOffset, Rectangle* rect) {
    local_persist u32 COLOR_BG = get_color(60, 60, 60);
    fill_color(bitmapBuffer, COLOR_BG);

    local_persist u16 RECT_SIZE = 40;
    local_persist u32 COLOR_FG = get_color(120, 200, 255);
    draw_horizontal_line_segment(bitmapBuffer, yOffset            , xOffset, xOffset + RECT_SIZE, COLOR_FG); // top
    draw_horizontal_line_segment(bitmapBuffer, yOffset + RECT_SIZE, xOffset, xOffset + RECT_SIZE, COLOR_FG); // bottom
    draw_vertical_line_segment  (bitmapBuffer, xOffset            , yOffset, yOffset + RECT_SIZE, COLOR_FG); // left
    draw_vertical_line_segment  (bitmapBuffer, xOffset + RECT_SIZE, yOffset, yOffset + RECT_SIZE, COLOR_FG); // right

}

struct Button_State {
    int halfTransistionCount;
    bool endedDown;
};

enum Button_Index {
   BUTTON_UP,
   BUTTON_DOWN,
   BUTTON_LEFT,
   BUTTON_RIGHT,

   BUTTON_COUNT
};

struct Keyboard_Input {
    bool isConnected;
    Button_State buttons[BUTTON_COUNT];
};

struct Mouse_Input {
    int x;
    int y;
    bool isButtonPressed; // any button on the mouse, for now
};

struct State {
    int xOffset;
    int yOffset;
};

struct Memory {
    bool isInitialized;
    u64 permanentStorageSize;
    void* permanentStorage; // should be cleared to zero at startup (For Win32, VirtualAlloc does this)
    u64 transientStorageSize;
    void* transientStorage; // should be cleared to zero at startup (For Win32, VirtualAlloc does this)
};

void update(Memory* memory, Keyboard_Input* keyboard, Mouse_Input* mouse, Offscreen_Bitmap_Buffer* bitmapBuffer) {

    assert(sizeof(State) <= memory->permanentStorageSize);
    State* state = (State*) memory->permanentStorage;
    if (!memory->isInitialized) {

        Rectangle* rect = (Rectangle*) memory->permanentStorage;
        rect->x = 100;
        rect->y = 500;
        rect->w = 400;
        rect->h = 120;
        rect->border_color = get_color(255, 0, 0);

        memory->isInitialized = true;
    }

    if (mouse->isButtonPressed) {
        state->xOffset = mouse->x;
        state->yOffset = mouse->y;
    } else {
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

    //draw_weird_gradient(bitmapBuffer, state->xOffset, state->yOffset);
    //draw_test_color_bands(bitmapBuffer);
    //draw_crosshair(bitmapBuffer, state->xOffset, state->yOffset);
    //draw_grid(bitmapBuffer, state->xOffset, state->yOffset);

    draw_rect(bitmapBuffer, state->xOffset + 20, state->yOffset + 40, (Rectangle*) memory->permanentStorage);

    // TODO draw arbitrary points, then preserve their position while panning

}

} // namespace Render

