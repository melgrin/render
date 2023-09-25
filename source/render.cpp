// TODO
// - zoom
// - circle/oval
// - ??

#include "render.h"
#include "platform.h"

#define PI32 3.14159265f

#include <stdlib.h> // rand
#include <time.h> // time for srand
#include <math.h> // sin, cos
#include <string.h> // strlen

global const int BLUE_BIT_OFFSET = 0;
global const int GREEN_BIT_OFFSET = 8;
global const int RED_BIT_OFFSET = 16;

static u32 get_color(u8 red, u8 green, u8 blue) {
    u32 color = red << RED_BIT_OFFSET | green << GREEN_BIT_OFFSET | blue << BLUE_BIT_OFFSET;
    return color;
}

static void get_rgb(u8* red, u8* green, u8* blue, u32 color) {
    u8 r = (color >> RED_BIT_OFFSET  ) & 0xFF;
    u8 g = (color >> GREEN_BIT_OFFSET) & 0xFF;
    u8 b = (color >> BLUE_BIT_OFFSET ) & 0xFF;
    *red = r;
    *green = g;
    *blue = b;
}

namespace Render {

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

void fill_pixel(Offscreen_Bitmap_Buffer* bitmapBuffer, int x, int y, u32 color) {
    if (x >= 0 && x < bitmapBuffer->width &&
        y >= 0 && y < bitmapBuffer->height) {
        u8* row = (u8*) bitmapBuffer->memory;
        row += bitmapBuffer->pitch * y;
        u32* pixel = (u32*) row;
        pixel += x;
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

void rotate_point(Vector2* p, int x, int y, f32 theta) {

    if (theta == 0) return;

    f32 s = sinf(theta);
    f32 c = cosf(theta);

    // translate point back to origin
    p->x -= x;
    p->y -= y;

    // rotate point
    f32 xnew = p->x * c - p->y * s;
    f32 ynew = p->x * s + p->y * c;

    // translate point back
    p->x = xnew + x;
    p->y = ynew + y;
}

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

void draw_line(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yOffset, f32 theta, int xRotationOrigin, int yRotationOrigin, const Line* line) {

    // FIXME it's just a copy...either do all of this per pixel with no copies, or just bulk copy all of the objects to a block that's then morphed, rather than doing it one at a time on the stack several functions down.  Or maybe at least reuse the memory instead of using the stack.
    Line lineR = *line;
    rotate_point(&lineR.start, xRotationOrigin, yRotationOrigin, theta);
    rotate_point(&lineR.end  , xRotationOrigin, yRotationOrigin, theta);

    {
        const Vector2* p0;
        const Vector2* p1;

        if (lineR.start.x > lineR.end.x) {
            p0 = &lineR.end;
            p1 = &lineR.start;
        } else {
            p0 = &lineR.start;
            p1 = &lineR.end;
        }

        f32 d = p1->x - p0->x;

        //for (int x = p0->x; x <= p1->x; ++x) {
        for (f32 xf = p0->x; xf <= p1->x; xf += 1.0f) {
            f32 mu = (xf - p0->x) / d;
            f32 yf = linear_interpolate(p0->y, p1->y, mu);
            int yi = (int) (yf + 0.5);
            int xi = (int) (xf + 0.5);
            fill_pixel(bitmapBuffer, xi + xOffset, yi + yOffset, line->color);
        }
    }
    {
        const Vector2* p0;
        const Vector2* p1;

        if (lineR.start.y > lineR.end.y) {
            p0 = &lineR.end;
            p1 = &lineR.start;
        } else {
            p0 = &lineR.start;
            p1 = &lineR.end;
        }

        f32 d = p1->y - p0->y;

        //for (int y = p0->y; y <= p1->y; ++y) {
        for (f32 yf = p0->y; yf <= p1->y; yf += 1.0f) {
            f32 mu = (yf - p0->y) / d;
            f32 xf = linear_interpolate(p0->x, p1->x, mu);
            int xi = (int) (xf + 0.5);
            int yi = (int) (yf + 0.5);
            fill_pixel(bitmapBuffer, xi + xOffset, yi + yOffset, line->color);
        }
    }
}

void draw_rect(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yOffset, f32 theta, int xRotationOrigin, int yRotationOrigin, Rectangle* rect) {
    
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

    draw_line(bitmapBuffer, xOffset, yOffset, theta, xRotationOrigin, yRotationOrigin, &top);
    draw_line(bitmapBuffer, xOffset, yOffset, theta, xRotationOrigin, yRotationOrigin, &bottom);
    draw_line(bitmapBuffer, xOffset, yOffset, theta, xRotationOrigin, yRotationOrigin, &left);
    draw_line(bitmapBuffer, xOffset, yOffset, theta, xRotationOrigin, yRotationOrigin, &right);
}

void draw_crosshair(Offscreen_Bitmap_Buffer* bitmapBuffer, int x, int y, int length, u32 color) {

    Line v, h;
    int l = length/2;

    h.start.x = f32(x - l);
    h.end  .x = f32(x + l);
    h.start.y = f32(y);
    h.end  .y = f32(y);

    v.start.x = f32(x);
    v.end  .x = f32(x);
    v.start.y = f32(y - l);
    v.end  .y = f32(y + l);

    v.color = color;
    h.color = color;

    draw_line(bitmapBuffer, 0, 0, 0, 0, 0, &v);
    draw_line(bitmapBuffer, 0, 0, 0, 0, 0, &h);
}

void draw_circle(Offscreen_Bitmap_Buffer* bitmapBuffer, int xOffset, int yOffset, f32 theta, int xRotationOrigin, int yRotationOrigin, Circle* circle, u32 color) {
    
    // probably want to make this higher resolution
    // or maybe make it proportional to the radius
    local_persist const f32 step = 3.6f * PI32/180.0f;

    Vector2 v;

    u8 r, g, b;
    get_rgb(&r, &g, &b, color); // don't think this is right (brown -> green?), but at least I get the fade
    for (f32 i = 0.0; i < 2*PI32; i += step) {

        f32 s = sinf(i);
        f32 c = cosf(i);

        f32 xr = circle->radius * c;
        f32 yr = circle->radius * s;

        v.x = circle->center.x + xr;
        v.y = circle->center.y + yr;

        rotate_point(&v, xRotationOrigin, yRotationOrigin, theta);

        int xi = int(v.x) + xOffset;
        int yi = int(v.y) + yOffset;

        u8 ii = u8(100*(i/(2*PI32)));
        u32 _color = get_color(r + ii, g + ii, b + ii);

        fill_pixel(bitmapBuffer, xi, yi, _color);
    }

}

#if ZOOM
// FIXME Can't zoom on arbitrary point.  It wouldn't make sense after the whole rotation around the testLine debacle.
// FIXME FIXME morphs the points
void scale_point(Vector2* p, int xOffset, int yOffset, int xMouse, int yMouse, int scrollFactor) {
    //p->x = p->x - xOffset + scrollFactor;
    //p->y = p->y - yOffset + scrollFactor;
    //p->x = p->x + scrollFactor * 10;
    //p->y = p->y + scrollFactor * 10;

    //f32 diff = f32(mouse) - p.x;
    //f32 direction = diff / abs(diff);
    //p.x += diff * 10;
    //if (mouse - p > 0) ++;
    //else if (mouse - p < 0) --;

    int xScroll = scrollFactor * 10;
    int yScroll = scrollFactor * 10;
    if (xMouse > p->x) xScroll *= -1;
    if (yMouse > p->y) yScroll *= -1;

    p->x += xScroll;
    p->y += yScroll;
}
#endif

void draw_character(Offscreen_Bitmap_Buffer* bitmapBuffer, Font* font, char c, int x_left, int y_baseline, u32 color) {
    int y_top = y_baseline - font->rows_per_char;
    int atlas_index = (int) c;
    if (atlas_index >= 0 && (size_t) atlas_index < arrayCount(font->atlas)) {
        const char* char_pixmap = font->atlas[atlas_index];
        for (int row = 0; row < font->rows_per_char; ++row) {
            for (int col = 0; col < font->cols_per_char; ++col) {
                int char_index = row * font->cols_per_char + col;
                char p = char_pixmap[char_index];
                if (p == font->char_pixmap_populated) {
                    fill_pixel(bitmapBuffer, x_left + col, y_top + row, color);
                }
            }
        }
    } else {
        for (int x = 0; x < font->cols_per_char; ++x) {
            for (int y = 0; y < font->rows_per_char; ++y) {
                fill_pixel(bitmapBuffer, x_left + x, y_top + y, color);
            }
        }
    }
}

void draw_text(Offscreen_Bitmap_Buffer* bitmapBuffer, Font* font, int x, int y, const char* text) {
    int xOffset = 0;
    for (const char* c = text; *c; ++c) {
        draw_character(bitmapBuffer, font, *c, x + xOffset, y, 0x00ff00);
        xOffset += font->cols_per_char + 1;
    }
}

void update(Memory* memory, Input* input, Offscreen_Bitmap_Buffer* bitmapBuffer) {

    assert(sizeof(State) <= memory->permanentStorageSize);
    State* state = (State*) memory->permanentStorage;
    local_persist Crosshair* next_ch = 0;
    local_persist Crosshair* first_ch = 0;
    local_persist u32 num_ch = 0;
    bool generate_shapes = false;
    if (!memory->isInitialized) {
        generate_shapes = true;
        srand((int)time(0)); // seed
        state->background = BACKGROUND_BLANK;
        state->textVisible = true;
        if (!init_font(&state->font)) return;
        memory->isInitialized = true;
    }


    {
        const Button_State* testButton = &input->keyboard.buttons[KEYBOARD_BUTTON_U];
        if (testButton->downCount > 0) {
            Platform::report_error("The 'test that report_error works' button was pushed!  ...Did it work?");
        }
    }

    {
        const Button_State* refreshButton = &input->keyboard.buttons[KEYBOARD_BUTTON_R];
        if (refreshButton->downCount > 0) {
            generate_shapes = true;
        }
    }

    if (generate_shapes) {

        assert(memory->isInitialized == true);

        u8* p = (u8*) memory->permanentStorage;
        p += sizeof(State);

        u32 rect_color = get_color(222, 126, 45);

        state->rectangles = (Rectangle*) p;
        state->rectanglesCount = 20;
        for (int i = 0; i < state->rectanglesCount; ++i) {
            Rectangle* rect = (Rectangle*) p;
            f32 x = (f32) (rand() % 1000);
            f32 y = (f32) (rand() %  500);
            f32 w = (f32) (rand() %  100 + 1);
            f32 h = (f32) (rand() %  100 + 1);
            rect->nw.x = x;
            rect->nw.y = y;
            rect->ne.x = x + w;
            rect->ne.y = y;
            rect->sw.x = x;
            rect->sw.y = y + h;
            rect->se.x = x + w;
            rect->se.y = y + h;
            rect->border_color = rect_color;
            p += sizeof(Rectangle);
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


        state->circles = (Circle*) p;
        state->circlesCount = 10;
        for (int i = 0; i < state->circlesCount; ++i) {
            Circle* circle = (Circle*) p;
            circle->center.x = 400.0f + f32(i*4);
            circle->center.y = 400.0f + f32(i*3);
            circle->radius = 50.0f;
            p += sizeof(Circle);
        }

        // FIXME these will be (kind of) broken if I regen
        next_ch = (Crosshair*) p;
        first_ch = next_ch;
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

    {
        int i = MOUSE_BUTTON_MIDDLE;

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

    f32 thetaFromMouseButtons = 0;
    {
        Button_State* left  = &input->mouse.buttons[MOUSE_BUTTON_LEFT ];
        Button_State* right = &input->mouse.buttons[MOUSE_BUTTON_RIGHT];

        local_persist const f32 STEP = 10.0f * PI32/180.0f;
        //if (left->changed && left->endedDown) {
        if (left->endedDown) {
            thetaFromMouseButtons -= STEP;
        }
        //if (right->changed && right->endedDown) { 
        if (right->endedDown) { 
            thetaFromMouseButtons += STEP;
        }
    }
    state->rotationAngle += thetaFromMouseButtons;
    //Platform::DEBUG_display("rotate %.2f", state->rotationAngle*180/PI32);

    f32 thetaFromKeyboardButtons = 0;
    {
        Button_State* q = &input->keyboard.buttons[KEYBOARD_BUTTON_Q];
        Button_State* e = &input->keyboard.buttons[KEYBOARD_BUTTON_E];
        local_persist const f32 STEP = 10.0f * PI32/180.0f;
        thetaFromKeyboardButtons -= q->downCount * STEP;
        thetaFromKeyboardButtons += e->downCount * STEP;
    }
    state->rotationAngle += thetaFromKeyboardButtons;


    if (state->mouseDragging) {
        // old version that panned wrong at any non-zero rotationAngle
        //state->xOffset += cursor->change.x;
        //state->yOffset += cursor->change.y;

        f32 s = sinf(state->rotationAngle * -1.0f);
        f32 c = cosf(state->rotationAngle * -1.0f);

        f32 x = f32(cursor->change.x);
        f32 y = f32(cursor->change.y);

        f32 dx = x * c - y * s;
        f32 dy = x * s + y * c;

        state->xOffset += int(dx); // XXX f32?
        state->yOffset += int(dy); // XXX f32?

        state->xBackgroundOffset += cursor->change.x;
        state->yBackgroundOffset += cursor->change.y;
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

    if (input->mouse.wheel.delta != 0) {
        int factor = (int) state->blankBackgroundColorFactor;
        factor += 10 * input->mouse.wheel.delta;
        if (factor < 0) { factor = 0; }
        else if (factor > 255) { factor = 255; }
        state->blankBackgroundColorFactor  = (u8) factor;
    }

    if (input->keyboard.buttons[KEYBOARD_BUTTON_1].endedDown) {
        state->background = BACKGROUND_BLANK;
    } else if (input->keyboard.buttons[KEYBOARD_BUTTON_2].endedDown) {
        state->background = BACKGROUND_GRADIENT;
    } else if (input->keyboard.buttons[KEYBOARD_BUTTON_3].endedDown) {
        state->background = BACKGROUND_BANDS;
    }

    if (state->background == BACKGROUND_BLANK) {
        u8 bg = state->blankBackgroundColorFactor;
        u32 color = get_color(bg, bg, bg);
        fill_color(bitmapBuffer, color);
    } else if (state->background == BACKGROUND_GRADIENT) {
        draw_weird_gradient(bitmapBuffer, state->xBackgroundOffset, state->yBackgroundOffset);
    } else if (state->background == BACKGROUND_BANDS) {
        draw_test_color_bands(bitmapBuffer);
    }

    if (input->keyboard.buttons[KEYBOARD_BUTTON_G].endedDown) {
        draw_grid(bitmapBuffer, state->xBackgroundOffset, state->yBackgroundOffset);
    }

    if (input->keyboard.buttons[KEYBOARD_BUTTON_T].changed &&
        input->keyboard.buttons[KEYBOARD_BUTTON_T].endedDown) {
        state->textVisible = !state->textVisible;
    }

    if (state->textVisible) {
        //draw_horizontal_line_segment(bitmapBuffer, 10, 10, 100, 0xffff00);
        draw_text(bitmapBuffer, &state->font, 10, 10, "hello ... aboba");
        draw_text(bitmapBuffer, &state->font, 10, 20, "abcdefghijklmnopqrstuvwxyz");
        draw_text(bitmapBuffer, &state->font, 10, 30, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        draw_text(bitmapBuffer, &state->font, 10, 40, "1234567890");
        draw_text(bitmapBuffer, &state->font, 10, 50, "-=_+!@#$%^&*()`~,./\\|;'[]<>?:\"{}");

        draw_text(bitmapBuffer, &state->font, 10, 70, "Here's a real sentence with punctuation.  You might say it's a \"demo\"!?");
    }

    //draw_crosshair(bitmapBuffer, state->xOffset, state->yOffset);

    Vector2i viewCenter;
    viewCenter.x = bitmapBuffer->width  / 2;
    viewCenter.y = bitmapBuffer->height / 2;

    int xRotationOrigin = viewCenter.x - state->xOffset;
    int yRotationOrigin = viewCenter.y - state->yOffset;

    Crosshair* ch = first_ch;
    local_persist u32 COLOR_PAN_CH = get_color(200, 150, 30);
    for (u32 i = 0; i < num_ch; ++i) {
        draw_crosshair(bitmapBuffer, ch->x, ch->y, 20, COLOR_PAN_CH);
        ++ch;
    }

    local_persist u32 COLOR_CH = get_color(255, 255, 255);
    draw_crosshair(bitmapBuffer, viewCenter.x, viewCenter.y, 100, COLOR_CH);

    for (int i = 0; i < state->rectanglesCount; ++i) {
        draw_rect(bitmapBuffer, state->xOffset, state->yOffset, state->rotationAngle, xRotationOrigin, yRotationOrigin, &state->rectangles[i]);
    }

    for (int i = 0; i < state->linesCount; ++i) {
        Line* line = &state->lines[i];
        draw_line(bitmapBuffer, state->xOffset, state->yOffset, state->rotationAngle, xRotationOrigin, yRotationOrigin, line);
        //draw_line(bitmapBuffer, state->xOffset, state->yOffset, line);
        //draw_line_old(bitmapBuffer, state->xOffset + 50, state->yOffset + 50, line); // + 50 for debugging compared to new draw_line
    }

    local_persist u32 COLOR_CIRCLE = get_color(255, 180, 100);
    for (int i = 0; i < state->circlesCount; ++i) {
        draw_circle(bitmapBuffer, state->xOffset, state->yOffset, state->rotationAngle, xRotationOrigin, yRotationOrigin, &state->circles[i], COLOR_CIRCLE);
    }

#if ZOOM
    if (input->mouse.wheel.delta != 0) {
        //Platform::DEBUG_printf("mouse wheel delta: %d\n", input->mouse.wheel.delta);
        for (int i = 0; i < state->rectanglesCount; ++i) {
            for (int j = 0; j < 4; ++j) {
                scale_point(&state->rectangles[i].points[j],
                    state->xOffset,
                    state->yOffset,
                    input->mouse.cursor.position.x,
                    input->mouse.cursor.position.y,
                    input->mouse.wheel.delta);
                    //TODO
                    //input->mouse.wheel.position.x - xOffset,
                    //input->mouse.wheel.position.y - yOffset);
            }
        }
    }
#endif


}

} // namespace Render

/* line angle
void xx() {
    f32 x = state->testLine->end.x - state->testLine->start.x;
    f32 y = state->testLine->end.y - state->testLine->start.y;
    f32 length = sqrtf(powf(x,2) + powf(y,2));
    f32 angle = atan2f(y,x);
    if (y < 0) angle += 2*PI32;
}
*/


/* FIXME: this draws lines that don't look like they're at x==10. they're in a different spot than from fill_pixel
        {Line tmp = {{10, 10}, {100, 10}, 0xff0000};
        draw_line(bitmapBuffer, 10, 10, 0, 0, 0, &tmp);}
        {Line tmp = {{10, 1}, {100, 1}, 0xffff00};
        draw_line(bitmapBuffer, 10, 10, 0, 0, 0, &tmp);}
*/
