#ifndef RENDERDEV_TEXT_H
#define RENDERDEV_TEXT_H

#include "common.h"

struct Font {
    u16 rows_per_char;
    u16 cols_per_char;
    char char_pixmap_populated;
    char char_pixmap_unpopulated;
    typedef const char* Atlas[128];
    Atlas atlas;
};

bool init_font(Font* out);
//void draw_character(Offscreen_Bitmap_Buffer* bitmapBuffer, Font* font, char c, int x_left, int y_baseline, u32 color);
//void draw_text(Offscreen_Bitmap_Buffer* bitmapBuffer, Font* font, int x, int y, const char* text);

#endif // RENDERDEV_TEXT_H
