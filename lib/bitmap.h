#ifndef BITMAP_H
#define BITMAP_H

#include <windows.h>

HBITMAP bitmap_tinted(HBITMAP src, COLORREF tint);
HBITMAP bitmap_flipped_h(HBITMAP src);

#endif