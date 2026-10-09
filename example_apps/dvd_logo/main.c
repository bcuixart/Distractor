#include <windows.h>
#include "window.h"

// gcc -o main.exe main.c window.c -lgdi32

void on_tick(Window *win)
{
    static int dx = 5;
    static int dy = 5;

    win->x += dx;
    win->y += dy;
    if (win->x <= 0 || win->x + win->w >= GetSystemMetrics(SM_CXSCREEN)) dx = -dx;
    if (win->y <= 0 || win->y + win->h >= GetSystemMetrics(SM_CYSCREEN)) dy = -dy;

    window_move_to(win, win->x, win->y);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Unused parameters (to avoid compiler warnings)
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    if (!init_window_module(hInstance, "Test Window")) return 1;

    HBITMAP hBitmap = (HBITMAP) LoadImageW(hInstance, L"DVD_LOGO", IMAGE_BITMAP, 0, 0, 0);
    if (hBitmap == NULL) return 2; 

    Window win = {0};
    if (!window_create(&win, 100, 100, 110, 65)) return 3;
    window_set_tick_callback(&win, on_tick);
    window_set_bitmap(&win, hBitmap);

    return window_run();
}