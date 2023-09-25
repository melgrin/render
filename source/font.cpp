#include "font.h"
#include "platform.h"
#include <string.h>

#define ATLAS_ROWS_PER_CHAR 6
#define ATLAS_COLS_PER_CHAR 5
#define ATLAS_CHAR_PIXMAP_POPULATED   "O"
#define ATLAS_CHAR_PIXMAP_UNPOPULATED "_"

// 'o' is stupid tall to match 'e', which really can't be any shorter
// there's no concept of "below baseline", but there should be
// for some characters like 'x' and 'v', it feels like I don't have enough fidelity

#define O ATLAS_CHAR_PIXMAP_POPULATED
#define _ ATLAS_CHAR_PIXMAP_UNPOPULATED

const static Font::Atlas ATLAS = {
//static const char* ATLAS[128] = {

_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 0
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 1
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 2
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 3
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 4
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 5
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 6
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 7
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 8
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 9
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 10
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 11
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 12
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 13
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 14
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 15
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 16
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 17
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 18
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 19
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 20
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 21
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 22
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 23
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 24
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 25
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 26
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 27
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 28
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 29
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 30
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 31

_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _, // 32 space

// 33 !
_ _ O _ _
_ _ O _ _
_ _ O _ _
_ _ O _ _
_ _ _ _ _
_ _ O _ _,

// 34 "
_ O _ O _
_ O _ O _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _,

// 35 #
_ _ _ _ _
_ O _ O _
O O O O O
_ O _ O _
O O O O O
_ O _ O _,

// 36 $ ....
//_ _ O _ _
//_ _ O O _
//_ O _ _ _
//_ _ O O _
//_ _ _ _ O
//_ _ O O _,
_ _ _ _ _
_ _ O O _
_ O _ _ _
O O O O _
_ O _ _ _
_ _ O O _,

// 37 %
_ _ _ _ _
_ O _ _ O
_ _ _ O _
_ _ O _ _
_ O _ _ _
O _ _ O _,

// 38 &
_ O O _ _
O _ _ _ _
O _ _ _ _
_ O _ O _
O _ O _ _
_ O _ O _,

// 39 '
_ _ O _ _
_ _ O _ _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _,

// 40 (
_ _ O _ _
_ O _ _ _
_ O _ _ _
_ O _ _ _
_ O _ _ _
_ _ O _ _,

// 41 )
_ _ O _ _
_ _ _ O _
_ _ _ O _
_ _ _ O _
_ _ _ O _
_ _ O _ _,

// 42 *
_ _ _ _ _
_ O _ O _
_ _ O _ _
_ O _ O _
_ _ _ _ _
_ _ _ _ _,

// 43 +
_ _ _ _ _
_ _ _ _ _
_ _ O _ _
_ O O O _
_ _ O _ _
_ _ _ _ _,

// 44 ,
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _
_ _ O _ _
_ O _ _ _,

// 45 -
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _
_ O O O _
_ _ _ _ _
_ _ _ _ _,

// 46 .
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _
_ _ O _ _,

// 47 /
_ _ _ _ _
_ _ _ _ O
_ _ _ O _
_ _ O _ _
_ O _ _ _
O _ _ _ _,

// 48 0
_ O O _ _
O _ _ O _
O _ O O _
O O _ O _
O _ _ O _
_ O O _ _,

// 49 1
_ _ O _ _
_ O O _ _
_ _ O _ _
_ _ O _ _
_ _ O _ _
_ O O O _,

// 50 2
_ O O _ _
O _ _ O _
_ _ _ O _
_ _ O _ _
_ O _ _ _
O O O O _,

// 51 3
O O O _ _
_ _ _ O _
_ O O _ _
_ _ _ O _
O _ _ O _
_ O O _ _,

// 52 4
_ _ _ O _
_ _ O O _
_ O _ O _
O O O O O
_ _ _ O _
_ _ _ O _,

// 53 5
O O O O _
O _ _ _ _
O O O _ _
_ _ _ O _
_ _ _ O _
O O O _ _,

// 54 6
_ O O O _
O _ _ _ _
O O O O _
O _ _ _ O
O _ _ _ O
_ O O O _,

// 55 7
O O O O O
_ _ _ _ O
_ _ _ O _
_ _ O _ _
_ O _ _ _
O _ _ _ _,

// 56 8
_ O O _ _
O _ _ O _
_ O O _ _
O _ _ O _
O _ _ O _
_ O O _ _,

// 57 9
_ O O O _
O _ _ _ O
O _ _ _ O
_ O O O O
_ _ _ _ O
_ O O O _,

// 58 :
_ _ _ _ _
_ _ _ _ _
_ _ O _ _
_ _ _ _ _
_ _ O _ _
_ _ _ _ _,

// 59 ;
_ _ _ _ _
_ _ _ _ _
_ _ O _ _
_ _ _ _ _
_ _ O _ _
_ O _ _ _,

// 60 <
_ _ _ _ _
_ _ _ O _
_ _ O _ _
_ O _ _ _
_ _ O _ _
_ _ _ O _,

// 61 =
_ _ _ _ _
_ _ _ _ _
O O O O _
_ _ _ _ _
O O O O _
_ _ _ _ _,

// 62 >
_ _ _ _ _
_ O _ _ _
_ _ O _ _
_ _ _ O _
_ _ O _ _
_ O _ _ _,

// 63 ?
_ _ O _ _
_ O _ O _
_ _ _ O _
_ _ O _ _
_ _ _ _ _
_ _ O _ _,

// 64 @
_ O O O _
O _ _ _ O
O _ O O O
O _ O O O
O _ _ _ _
_ O O O _,

// 65 A
_ O O _ _
O _ _ O _
O _ _ O _
O O O O _
O _ _ O _
O _ _ O _,

// 66 B
O O _ _ _
O _ O _ _
O O O _ _
O _ _ O _
O _ _ O _
O O O _ _,

// 67 C
_ O O O _
O _ _ _ _
O _ _ _ _
O _ _ _ _
O _ _ _ _
_ O O O _,

// 68 D
O O O _ _
O _ _ O _
O _ _ O _
O _ _ O _
O _ _ O _
O O O _ _,

// 69 E
O O O O _
O _ _ _ _
O O O _ _
O _ _ _ _
O _ _ _ _
O O O O _,

// 70 F
O O O O _
O _ _ _ _
O O O _ _
O _ _ _ _
O _ _ _ _
O _ _ _ _,

// 71 G
_ O O O _
O _ _ _ _
O _ _ _ _
O _ O O O
O _ _ _ O
_ O O O _,

// 72 H
O _ _ O _
O _ _ O _
O O O O _
O _ _ O _
O _ _ O _
O _ _ O _,

// 73 I
_ O O O _
_ _ O _ _
_ _ O _ _
_ _ O _ _
_ _ O _ _
_ O O O _,

// 74 J
_ O O O _
_ _ O _ _
_ _ O _ _
_ _ O _ _
O _ O _ _
_ O _ _ _,

// 75 K
O _ _ O _
O _ O _ _
O O _ _ _
O _ O _ _
O _ _ O _
O _ _ _ O,

// 76 L
O _ _ _ _
O _ _ _ _
O _ _ _ _
O _ _ _ _
O _ _ _ _
O O O O _,

// 77 M
O _ _ _ O
O O _ O O
O _ O _ O
O _ _ _ O
O _ _ _ O
O _ _ _ O,

// 78 N
O _ _ _ O
O O _ _ O
O _ O _ O
O _ _ O O
O _ _ _ O
O _ _ _ O,

// 79 O
_ O O O _
O _ _ _ O
O _ _ _ O
O _ _ _ O
O _ _ _ O
_ O O O _,

// 80 P
O O O _ _
O _ _ O _
O _ _ O _
O O O _ _
O _ _ _ _
O _ _ _ _,

// 81 Q
_ O O _ _
O _ _ O _
O _ _ O _
O _ O O _
O _ _ O _
_ O O _ O,

// 82 R
O O O _ _
O _ _ O _
O _ _ O _
O O O _ _
O _ _ O _
O _ _ O _,

// 83 S
_ O O O _
O _ _ _ _
_ O O O _
_ _ _ _ O
_ _ _ _ O
_ O O O _,

// 84 T
O O O O O
_ _ O _ _
_ _ O _ _
_ _ O _ _
_ _ O _ _
_ _ O _ _,

// 85 U
O _ _ _ O
O _ _ _ O
O _ _ _ O
O _ _ _ O
O _ _ _ O
_ O O O _,

// 86 V
O _ _ _ O
O _ _ _ O
O _ _ _ O
O _ _ _ O
_ O _ O _
_ _ O _ _,

// 87 W
O _ _ _ O
O _ _ _ O
O _ _ _ O
O _ O _ O
O _ O _ O
_ O _ O _,

// 88 X
_ _ _ _ _
O _ _ _ O
_ O _ O _
_ _ O _ _
_ O _ O _
O _ _ _ O,

// 89 Y
O _ _ _ O
O _ _ _ O
_ O _ O _
_ _ O _ _
_ _ O _ _
_ _ O _ _,

// 90 Z
O O O O O
_ _ _ O _
_ _ O _ _
_ O _ _ _
O _ _ _ _
O O O O O,

// 91 [
_ O O O _
_ O _ _ _
_ O _ _ _
_ O _ _ _
_ O _ _ _
_ O O O _,

// 92 '\'
_ _ _ _ _
O _ _ _ _
_ O _ _ _
_ _ O _ _
_ _ _ O _
_ _ _ _ O,

// 93 ]
_ O O O _
_ _ _ O _
_ _ _ O _
_ _ _ O _
_ _ _ O _
_ O O O _,

// 94 ^
_ _ O _ _
_ O _ O _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _,

// 95 _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _
O O O O O,

// 96 `
_ O _ _ _
_ _ O _ _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _,

// 97 a
_ _ _ _ _
_ O O _ _
_ _ _ O _
_ O O O _
O _ _ O _
_ O O O _,

O _ _ _ _
O _ _ _ _
O O O _ _
O _ _ O _
O _ _ O _
O O O _ _,

_ _ _ _ _
_ _ _ _ _
_ O O O _
O _ _ _ _
O _ _ _ _
_ O O O _,

_ _ _ O _
_ _ _ O _
_ O O O _
O _ _ O _
O _ _ O _
_ O O O _,

_ _ _ _ _
_ O O _ _
O _ _ O _
O O O O _
O _ _ _ _
_ O O _ _,

_ _ O O _
_ O _ _ _
_ O _ _ _
O O O _ _
_ O _ _ _
_ O _ _ _,

_ _ _ _ _
_ O O O _
O _ _ O _
_ O O O _
_ _ _ O _
_ O O _ _,

O _ _ _ _
O _ _ _ _
O O O _ _
O _ _ O _
O _ _ O _
O _ _ O _,

_ _ _ _ _
_ _ O _ _
_ _ _ _ _
_ _ O _ _
_ _ O _ _
_ _ O _ _,

_ _ _ O _
_ _ _ _ _
_ _ _ O _
_ _ _ O _
_ _ _ O _
_ O O _ _,

O _ _ _ _
O _ _ _ _
O _ O _ _
O O _ _ _
O _ O _ _
O _ _ O _,

_ O _ _ _
_ O _ _ _
_ O _ _ _
_ O _ _ _
_ O _ _ _
_ _ O _ _,

_ _ _ _ _
_ _ _ _ _
O O O O _
O _ O _ O
O _ O _ O
O _ _ _ O,

_ _ _ _ _
_ _ _ _ _
O O O _ _
O _ _ O _
O _ _ O _
O _ _ O _,

_ _ _ _ _
_ O O _ _
O _ _ O _
O _ _ O _
O _ _ O _
_ O O _ _,

_ _ _ _ _
O O O _ _
O _ _ O _
O O O _ _
O _ _ _ _
O _ _ _ _,

_ _ _ _ _
_ O O O _
O _ _ O _
_ O O O _
_ _ _ O _
_ _ _ O _,

_ _ _ _ _
_ _ _ _ _
_ O O O _
_ O _ _ _
_ O _ _ _
_ O _ _ _,

_ _ _ _ _
_ _ O O _
_ O _ _ _
_ _ O O _
_ _ _ _ O
_ _ O O _,

_ O _ _ _
_ O _ _ _
O O O _ _
_ O _ _ _
_ O _ _ _
_ _ O O _,

_ _ _ _ _
_ _ _ _ _
_ O _ _ O
_ O _ _ O
_ O _ _ O
_ _ O O O,

_ _ _ _ _
_ _ _ _ _
O _ _ O _
_ O _ O _
_ _ O O _
_ _ _ O _,

_ _ _ _ _
_ _ _ _ _
O _ _ _ O
O _ O _ O
O _ O _ O
_ O _ O O,

_ _ _ _ _
O _ _ _ O
_ O _ O _
_ _ O _ _
_ O _ O _
O _ _ _ O,

_ _ _ _ _
O _ _ O _
O _ _ O _
_ O O O _
_ _ _ O _
_ O O _ _,

_ _ _ _ _
_ _ _ _ _
O O O O _
_ _ O _ _
_ O _ _ _
O O O O _,

// 123 {
_ _ O _ _
_ O _ _ _
_ O _ _ _
O O _ _ _
_ O _ _ _
_ _ O _ _,

// 124 |
_ _ O _ _
_ _ O _ _
_ _ O _ _
_ _ O _ _
_ _ O _ _
_ _ O _ _,

// 125 }
_ _ O _ _
_ _ _ O _
_ _ _ O _
_ _ _ O O
_ _ _ O _
_ _ O _ _,

// 126 ~
_ _ _ _ _
_ _ _ _ _
_ O _ O _
O _ O _ _
_ _ _ _ _
_ _ _ _ _,

// 127 DEL
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _
_ _ _ _ _,

};

#undef O
#undef _

bool init_font(Font* out) {
    if (!out) return false;
    out->rows_per_char = ATLAS_ROWS_PER_CHAR;
    out->cols_per_char = ATLAS_COLS_PER_CHAR;
    out->char_pixmap_populated = ATLAS_CHAR_PIXMAP_POPULATED[0];
    out->char_pixmap_unpopulated = ATLAS_CHAR_PIXMAP_UNPOPULATED[0];
    memcpy(out->atlas, ATLAS, sizeof(Font::Atlas));
    {
        const size_t expected = ATLAS_ROWS_PER_CHAR * ATLAS_COLS_PER_CHAR;
        for (size_t i = 0; i < arrayCount(ATLAS); ++i) {
            const char* char_pixmap = ATLAS[i];
            size_t actual = strlen(char_pixmap);
            if (actual != expected) {
                Platform::report_error("Incorrect atlas entry size for entry %zu of %zu: expected %zu, got %zu\n",
                    i+1, arrayCount(ATLAS), expected, actual);
                return false;
            }
            //Platform::DEBUG_printf("atlas[%zu] strlen %zu\n", i, strlen(char_pixmap));
        }
    }
    return true;
}

