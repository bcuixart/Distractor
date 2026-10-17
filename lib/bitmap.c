#include <stdlib.h>
#include <stdint.h>
#include "bitmap.h"

#define ALPHA_COLOR 0x00FF00FFu

static uint32_t *get_pixels(HBITMAP src, BITMAPINFO *bmi)
{
    BITMAP bm;
    if (!GetObject(src, sizeof(bm), &bm)) return NULL;

    ZeroMemory(bmi, sizeof(*bmi));
    bmi->bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi->bmiHeader.biWidth       = bm.bmWidth;
    bmi->bmiHeader.biHeight      = bm.bmHeight;
    bmi->bmiHeader.biPlanes      = 1;
    bmi->bmiHeader.biBitCount    = 32;
    bmi->bmiHeader.biCompression = BI_RGB;

    uint32_t *pixels = malloc((size_t)bm.bmWidth * bm.bmHeight * sizeof(uint32_t));
    if (!pixels) return NULL;

    HDC hdc = GetDC(NULL);
    int ok = GetDIBits(hdc, src, 0, bm.bmHeight, pixels, bmi, DIB_RGB_COLORS);
    ReleaseDC(NULL, hdc);

    if (!ok) { free(pixels); return NULL; }
    return pixels;
}

static HBITMAP make_bitmap(const uint32_t *pixels, BITMAPINFO *bmi)
{
    int w = bmi->bmiHeader.biWidth;
    int h = bmi->bmiHeader.biHeight;

    HDC hdc = GetDC(NULL);
    HBITMAP result = CreateCompatibleBitmap(hdc, w, h);
    if (result && !SetDIBits(hdc, result, 0, h, pixels, bmi, DIB_RGB_COLORS))
    {
        DeleteObject(result);
        result = NULL;
    }
    ReleaseDC(NULL, hdc);

    return result;
}

HBITMAP bitmap_tinted(HBITMAP src, COLORREF tint)
{
    BITMAPINFO bmi;
    uint32_t *px = get_pixels(src, &bmi);
    if (!px) return NULL;

    int n = bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight;
    unsigned tr = GetRValue(tint), tg = GetGValue(tint), tb = GetBValue(tint);

    for (int i = 0; i < n; i++)
    {
        uint32_t c = px[i] & 0x00FFFFFF;
        if (c == ALPHA_COLOR) continue;

        unsigned r = (c >> 16) & 0xFF;
        unsigned g = (c >> 8)  & 0xFF;
        unsigned b =  c        & 0xFF;

        r = r * tr / 255;
        g = g * tg / 255;
        b = b * tb / 255;

        px[i] = (r << 16) | (g << 8) | b;
    }

    HBITMAP result = make_bitmap(px, &bmi);
    free(px);
    return result;
}

HBITMAP bitmap_flipped_h(HBITMAP src)
{
    BITMAPINFO bmi;
    uint32_t *px = get_pixels(src, &bmi);
    if (!px) return NULL;

    int w = bmi.bmiHeader.biWidth;
    int h = bmi.bmiHeader.biHeight;

    for (int y = 0; y < h; y++)
    {
        uint32_t *row = px + (size_t)y * w;
        for (int a = 0, b = w - 1; a < b; a++, b--)
        {
            uint32_t tmp = row[a];
            row[a] = row[b];
            row[b] = tmp;
        }
    }

    HBITMAP result = make_bitmap(px, &bmi);
    free(px);
    return result;
}