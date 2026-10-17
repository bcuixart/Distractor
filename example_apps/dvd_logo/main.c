#include <windows.h>
#include <stdlib.h>
#include "window.h"
#include "bitmap.h"

void print_error(const char *msg)
{
    DWORD errCode = GetLastError();
    LPVOID errMsg;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errMsg, 0, NULL);
    fprintf(stderr, "%s: %s\n", msg, (char *)errMsg);
    LocalFree(errMsg);
}

#define NUM_LOGOS 1

static void dvd_logo_on_tick(Window *win);
static void dvd_logo_on_click(Window *win, int x, int y);
static void dvd_logo_on_destroy(Window *win);

typedef struct 
{
    int x_abs_speed;
    int y_abs_speed;

    int x_dir_multiplier;
    int y_dir_multiplier;

    int color_index;
} Dvd_Logo;

#define NUM_COLORS 8
COLORREF colors[NUM_COLORS] = 
{
    RGB(190, 0, 255),
    RGB(0, 254, 255),
    RGB(255, 131, 0),
    RGB(0, 38, 255),
    RGB(255, 250, 1),
    RGB(255, 38, 0),
    RGB(255, 0, 139),
    RGB(0, 255, 38),
};
HBITMAP hBitmapsColored[NUM_COLORS] = {0};

int dvd_logo_create(Window *win, HBITMAP bitmap, int x, int y, int color_index)
{
    if (!win) return 0;

    Dvd_Logo *logo = (Dvd_Logo *)malloc(sizeof(Dvd_Logo));
    if (!logo) return 0;

    BITMAP bm;
    GetObject(bitmap, sizeof(bm), &bm);

    if (!window_create(win, x, y, bm.bmWidth / 10, bm.bmHeight / 10)) { free(logo); return 0; }
    window_set_tick_callback(win, dvd_logo_on_tick);
    window_set_click_callback(win, dvd_logo_on_click);
    window_set_destroy_callback(win, dvd_logo_on_destroy);
    window_set_bitmap(win, bitmap);

    logo->x_abs_speed = 5;
    logo->y_abs_speed = 5;
    logo->x_dir_multiplier = 1;
    logo->y_dir_multiplier = 1;
    logo->color_index = color_index;

    window_set_data(win, logo);

    return 1;
}

static void dvd_logo_on_tick(Window *win)
{
    Dvd_Logo *logo = (Dvd_Logo *)window_get_data(win);
    if (!logo) return;

    win->x += logo->x_abs_speed * logo->x_dir_multiplier;
    win->y += logo->y_abs_speed * logo->y_dir_multiplier;

    int collided_x = 0;
    int collided_y = 0;

    if (win->x <= 0) {
        logo->x_dir_multiplier = 1;
        collided_x = 1;
    }
    if (win->x + win->w >= GetSystemMetrics(SM_CXSCREEN)) {
        logo->x_dir_multiplier = -1;
        collided_x = 1;
    }

    if (win->y <= 0) {
        logo->y_dir_multiplier = 1;
        collided_y = 1;
    }
    if (win->y + win->h >= GetSystemMetrics(SM_CYSCREEN)) {
        logo->y_dir_multiplier = -1;
        collided_y = 1;
    }

    int color_index_old = logo->color_index;
    if (collided_x || collided_y) 
    {
        int color_index_new = color_index_old;
        do {
            color_index_new = rand() % NUM_COLORS;
        } while (color_index_new == color_index_old);
        window_set_bitmap(win, hBitmapsColored[color_index_new]);
        logo->color_index = color_index_new;
    }

    window_move_to(win, win->x, win->y);
}

static void dvd_logo_on_click(Window *win, int x, int y)
{
    (void)x;
    (void)y;

    window_destroy(win);
}

static void dvd_logo_on_destroy(Window *win)
{
    Dvd_Logo *logo = (Dvd_Logo *)window_get_data(win);
    if (logo) free(logo);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    srand((unsigned int)GetTickCount());

    // Unused parameters (to avoid compiler warnings)
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    if (!init_window_module(hInstance, "DVD")) { print_error("Failed to initialize window module"); return 1; };

    HBITMAP hBitmap = (HBITMAP) LoadImageW(hInstance, L"DVD_LOGO", IMAGE_BITMAP, 0, 0, 0);
    if (hBitmap == NULL) { print_error("Failed to load bitmap"); return 2; }

    for (int i = 0; i < NUM_COLORS; i++)
    {
        hBitmapsColored[i] = bitmap_tinted(hBitmap, colors[i]);
        if (hBitmapsColored[i] == NULL) { print_error("Failed to create colored bitmap"); return 3; }
    }

    Window logos[NUM_LOGOS] = {0};
    for (int i = 0; i < NUM_LOGOS; i++)
    {
        int x = rand() % (GetSystemMetrics(SM_CXSCREEN) - 100 + 1);
        int y = rand() % (GetSystemMetrics(SM_CYSCREEN) - 100 + 1);
        int c = rand() % NUM_COLORS;
        if (!dvd_logo_create(&logos[i], hBitmapsColored[c], x, y, c)) { print_error("Failed to create DVD logo"); return 4; }
    }

    int result = window_run();
    DeleteObject(hBitmap);
    for (int i = 0; i < NUM_COLORS; i++)
    {
        DeleteObject(hBitmapsColored[i]);
    }
    return result;
}