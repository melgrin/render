
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

#define PI32 3.14159265f

#include <stdlib.h> // rand
#include <time.h> // time for srand
#include <math.h> // sin, cos

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

struct Vector2 {
    //int x;
    //int y;
    f32 x;
    f32 y;
};

struct Vector2i {
    int x;
    int y;
};

struct Rectangle {
    f32 x;
    f32 y;
    f32 w;
    f32 h;
    u32 border_color;
    //u32 fill_color;
};

struct Rectangle_v2 {
    union {
        struct {
            Vector2 nw;
            Vector2 ne;
            Vector2 se;
            Vector2 sw;
        };
        Vector2 points[4];
    };
};

struct Line {
    Vector2 start;
    Vector2 end;
    u32 color;
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
    if (xOffset >= 0 && xOffset < bitmapBuffer->width &&
        yOffset >= 0 && yOffset < bitmapBuffer->height) {
        u8* row = (u8*) bitmapBuffer->memory;
        row += bitmapBuffer->pitch * yOffset;
        u32* pixel = (u32*) row;
        pixel += xOffset;
        *pixel = color;
    }
}

void draw_horizontal_line_segment(Offscreen_Bitmap_Buffer* bitmapBuffer, int yOffset, int xStart, int xEnd, u32 color) {

    if (yOffset >= bitmapBuffer->height) return;
    if (yOffset < 0) return;

    assert(xStart < xEnd);

    if (xStart < 0) xStart = 0;
    if (xEnd >= bitmapBuffer->width) xEnd = bitmapBuffer->width - 1;

    u8* row = (u8*) bitmapBuffer->memory;
    row += bitmapBuffer->pitch * yOffset;
    u32* pixel = (u32*) row;
    pixel += xStart;
    for (int x = xStart; x <= xEnd; ++x) {
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
    if (yEnd >= bitmapBuffer->height) yEnd = bitmapBuffer->height - 1;

    u8* row = (u8*) bitmapBuffer->memory;
    row += bitmapBuffer->pitch * yStart;

    for (int y = yStart; y <= yEnd; ++y) {
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

#if 0
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
#endif

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
    Vector2i position;
    Vector2i change;
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
    Rectangle_v2* rectangles_v2;
    int rectanglesCount_v2;
    Line* lines;
    int linesCount;
};

struct Memory {
    bool isInitialized;
    u64 permanentStorageSize;
    void* permanentStorage; // should be cleared to zero at startup (For Win32, VirtualAlloc does this)
    u64 transientStorageSize;
    void* transientStorage; // should be cleared to zero at startup (For Win32, VirtualAlloc does this)
};

typedef Vector2i Crosshair;

struct Input {
    Keyboard_Input keyboard;
    Mouse_Input mouse;
};

#if 0
int interpolate(const Vector2* a, const Vector2* b, int x/*, u32* color*/) {
    int y;
    if (a->x == b->x) {
        // avoid divide by zero
        //Platform::DEBUG_printf("interpolate: a.x == b.x, returning y = 0\n");
        //*color = get_color(0, 255, 0);
        y = 0;
    } else {
        // y = a->y + ((x - a->x) * ((b->y - a->y) / (b->x - a->x)));
        y = (a->y * (b->x - x) + b->y * (x - a->x)) / (b->x - a->x);
    }
    return y;
}
#endif

#if 0
void draw_line_old(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yOffset, const Line* line) {

    const Vector2* p0;
    const Vector2* p1;
    if (line->start.x > line->end.x) {
        p0 = &line->end;
        p1 = &line->start;
    } else {
        p0 = &line->start;
        p1 = &line->end;
    }

    for (int x = p0->x; x <= p1->x; ++x) {
        int y = interpolate(p0, p1, x/*, &line->color*/);
        fill_pixel(bitmapBuffer, x + xOffset, y + yOffset, get_color(255, 0, 255) /*line->color*/);
    }
}
#endif

// http://paulbourke.net/miscellaneous/interpolation/
// "Linear interpolation is the simplest method of getting values at positions in between the data points. The points are simply joined by straight line segments. Each segment (bounded by two data points) can be interpolated independently. The parameter mu defines where to estimate the value on the interpolated line, it is 0 at the first point and 1 and the second point. For interpolated values between the two points mu ranges between 0 and 1. Values of mu outside this range result in extrapolation."
// This seems to give the same effect as my original, but at least I understand it a little better.
f32 linear_interpolate(f32 y1, f32 y2, f32 mu) {
    f32 result = y1 * (1 - mu) + y2 * mu;
    return result;
}
f64 linear_interpolate(f64 y1, f64 y2, f64 mu) {
    f64 result = y1 * (1 - mu) + y2 * mu;
    return result;
}

void draw_line_linear_interpolate(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yOffset, const Line* line) {
    {
        const Vector2* p0;
        const Vector2* p1;

        if (line->start.x > line->end.x) {
            p0 = &line->end;
            p1 = &line->start;
        } else {
            p0 = &line->start;
            p1 = &line->end;
        }

        f32 d = p1->x - p0->x;

        //for (int x = p0->x; x <= p1->x; ++x) {
        for (f32 xf = p0->x; xf <= p1->x; xf += 1.0f) { // XXX ???
            f32 mu = (xf - p0->x) / d;
            f32 yf = linear_interpolate(p0->y, p1->y, mu);
            int yi = (int) (yf + 0.5); // XXX truncation/size conversion?
            int xi = (int) (xf + 0.5); // XXX truncation/size conversion?
            fill_pixel(bitmapBuffer, xi + xOffset, yi + yOffset, line->color);
        }
    }
    {
        const Vector2* p0;
        const Vector2* p1;

        if (line->start.y > line->end.y) {
            p0 = &line->end;
            p1 = &line->start;
        } else {
            p0 = &line->start;
            p1 = &line->end;
        }

        f32 d = p1->y - p0->y;

        //for (int y = p0->y; y <= p1->y; ++y) {
        for (f32 yf = p0->y; yf <= p1->y; yf += 1.0f) { // XXX ???
            f32 mu = (yf - p0->y) / d;
            f32 xf = linear_interpolate(p0->x, p1->x, mu);
            int xi = (int) (xf + 0.5); // XXX truncation/size conversion?
            int yi = (int) (yf + 0.5); // XXX truncation/size conversion?
            fill_pixel(bitmapBuffer, xi + xOffset, yi + yOffset, line->color);
        }
    }
}

void draw_rect_using_line_interp(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yOffset, Rectangle* rect) {

    // TODO? Really makes me think maybe I should do this just with Points/Vector2s.

    Line top, bottom, left, right;
    top   .color = get_color(255, 255, 255);
    bottom.color = get_color(255, 255,   0);
    left  .color = get_color(255,   0, 255);
    right .color = get_color(  0, 255, 255);

    top.start.x = rect->x;
    top.start.y = rect->y;
    top.end  .x = rect->x + rect->w;
    top.end  .y = rect->y;

    bottom.start.x = rect->x;
    bottom.start.y = rect->y + rect->h;
    bottom.end  .x = rect->x + rect->w;
    bottom.end  .y = rect->y + rect->h;

    left.start.x = rect->x;
    left.start.y = rect->y;
    left.end  .x = rect->x;
    left.end  .y = rect->y + rect->h;

    right.start.x = rect->x + rect->w;
    right.start.y = rect->y;
    right.end  .x = rect->x + rect->w;
    right.end  .y = rect->y + rect->h;

    draw_line_linear_interpolate(bitmapBuffer, xOffset, yOffset, &top);
    draw_line_linear_interpolate(bitmapBuffer, xOffset, yOffset, &bottom);
    draw_line_linear_interpolate(bitmapBuffer, xOffset, yOffset, &left);
    draw_line_linear_interpolate(bitmapBuffer, xOffset, yOffset, &right);
}

void draw_rect_v2(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yOffset, Rectangle_v2* rect) {
    
    Line top, bottom, left, right;
    top   .color = get_color(255, 255, 255);
    bottom.color = get_color(255, 255,   0);
    left  .color = get_color(255,   0, 255);
    right .color = get_color(  0, 255, 255);

    top.start = rect->nw;
    top.end   = rect->ne;

    bottom.start = rect->sw;
    bottom.end   = rect->se;

    left.start = rect->nw;
    left.end   = rect->sw;

    right.start = rect->ne;
    right.end   = rect->se;

    draw_line_linear_interpolate(bitmapBuffer, xOffset, yOffset, &top);
    draw_line_linear_interpolate(bitmapBuffer, xOffset, yOffset, &bottom);
    draw_line_linear_interpolate(bitmapBuffer, xOffset, yOffset, &left);
    draw_line_linear_interpolate(bitmapBuffer, xOffset, yOffset, &right);
}

#if 0
void _rotate_point_int_anchors(Vector2* p, int x, int y, f32 angleDegrees) {
    f32 angleRadians = angleDegrees*PI32/180.0f;
    f32 s = sin(angleRadians);
    f32 c = cos(angleRadians);

    // translate point back to origin
    p->x -= x;
    p->y -= y;

    // rotate point
    f32 fxnew = p->x * c - p->y * s;
    f32 fynew = p->x * s + p->y * c;

    int xnew = (int) (fxnew + 0.5); // XXX truncation/size conversion?
    int ynew = (int) (fynew + 0.5); // XXX truncation/size conversion?

    // translate point back
    p->x = xnew + x;
    p->y = ynew + y;
}
#endif

//void rotate_point(Vector2* p, f32 x, f32 y, f32 angleDegrees) {
void rotate_point(Vector2* p, int x, int y, f32 angleDegrees) {
    f32 angleRadians = angleDegrees*PI32/180.0f;
    f32 s = sinf(angleRadians);
    f32 c = cosf(angleRadians);

    // translate point back to origin
    p->x -= x;
    p->y -= y;

    // rotate point
    f32 fxnew = p->x * c - p->y * s;
    f32 fynew = p->x * s + p->y * c;

    f32 xnew = fxnew;
    f32 ynew = fynew;

    // translate point back
    p->x = xnew + x;
    p->y = ynew + y;
}

void update(Memory* memory, Input* input, Offscreen_Bitmap_Buffer* bitmapBuffer) {

    assert(sizeof(State) <= memory->permanentStorageSize);
    State* state = (State*) memory->permanentStorage;
    local_persist Crosshair* next_ch = 0;
    local_persist Crosshair* first_ch = 0;
    local_persist u32 num_ch = 0;
    if (!memory->isInitialized) {

        u8* p = (u8*) memory->permanentStorage;
        p += sizeof(State);

        srand((int)time(0)); // seed

        u32 rect_color = get_color(222, 126, 45);

        state->rectangles = (Rectangle*) p;
        state->rectanglesCount = 20;
        for (int i = 0; i < state->rectanglesCount; ++i) {
            Rectangle* rect = (Rectangle*) p;
            rect->x = (f32) (rand() % 1000);
            rect->y = (f32) (rand() %  500);
            rect->w = (f32) (rand() %  100 + 1);
            rect->h = (f32) (rand() %  100 + 1);
            rect->border_color = rect_color;
            p += sizeof(Rectangle);
        }

        // XXX tmp copy v1 to v2
        state->rectangles_v2 = (Rectangle_v2*) p;
        state->rectanglesCount_v2 = state->rectanglesCount;
        for (int i = 0; i < state->rectanglesCount_v2; ++i) {
            Rectangle_v2* rect = (Rectangle_v2*) p;
            Rectangle* src = &state->rectangles[i];
            rect->nw.x = src->x;
            rect->nw.y = src->y;
            rect->ne.x = src->x + src->w;
            rect->ne.y = src->y;
            rect->sw.x = src->x;
            rect->sw.y = src->y + src->h;
            rect->se.x = src->x + src->w;
            rect->se.y = src->y + src->h;
            p += sizeof(Rectangle_v2);
        }

        state->lines = (Line*) p;
        state->linesCount = 5;
        u32 line_color = get_color(255, 255, 255);
        for (int i = 0; i < state->linesCount; ++i) {
            Line* line = (Line*) p;
            // just some arbitrary placements
            line->start.x = (f32) (rand() % 300 + (bitmapBuffer->width / 2));
            line->start.y = (f32) (rand() % 300 + (bitmapBuffer->height / 2));
            line->end.x = (f32) (rand() % 300 + (bitmapBuffer->width / 2));
            line->end.y = (f32) (rand() % 300 + (bitmapBuffer->height / 2));
            line->color = line_color;
            p += sizeof(Line);
        }

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

    //XXX
    {
        Button_State* button = &input->mouse.buttons[MOUSE_BUTTON_RIGHT];

        if (button->changed && button->endedDown) {
            int xAnchor = input->mouse.cursor.position.x - state->xOffset;
            int yAnchor = input->mouse.cursor.position.y - state->yOffset;
            for (int i = 0; i < state->linesCount; ++i) {
                Line* line = &state->lines[i];
                rotate_point(&line->start, xAnchor, yAnchor, 10.0f);
                rotate_point(&line->end  , xAnchor, yAnchor, 10.0f);
            }
            for (int i = 0; i < state->rectanglesCount_v2; ++i) {
                for (int j = 0; j < 4; ++j) {
                    rotate_point(&state->rectangles_v2[i].points[j], xAnchor, yAnchor, 10);
                }
            }
        }
    }

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
        //draw_rect(bitmapBuffer, state->xOffset, state->yOffset, &state->rectangles[i]);
        draw_rect_using_line_interp(bitmapBuffer, state->xOffset + 2, state->yOffset + 2, &state->rectangles[i]);
        draw_rect_v2(bitmapBuffer, state->xOffset + 4, state->yOffset + 4, &state->rectangles_v2[i]);
    }

    for (int i = 0; i < state->linesCount; ++i) {
        Line* line = &state->lines[i];
        //draw_line(bitmapBuffer, state->xOffset, state->yOffset, line);

        draw_line_linear_interpolate(bitmapBuffer, state->xOffset, state->yOffset, line);
        //draw_line_old(bitmapBuffer, state->xOffset + 50, state->yOffset + 50, line); // + 50 for debugging compared to new draw_line
    }

    // TODO draw arbitrary points, then preserve their position while panning

}

} // namespace Render

