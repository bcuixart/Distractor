#include <windows.h>
#include <stdlib.h>
#include "window.h"

void print_error(const char *msg)
{
    DWORD errCode = GetLastError();
    LPVOID errMsg;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errMsg, 0, NULL);
    fprintf(stderr, "%s: %s\n", msg, (char *)errMsg);
    LocalFree(errMsg);
}

#define NUM_GRANDMAS 1
#define NUM_GRANDMA_IMAGES 8

#define GRANDMA_TICKS_PER_STEP 1
#define GRANDMA_TICKS_PER_IMAGE 6

HBITMAP hBitmaps[NUM_GRANDMA_IMAGES] = {0};

static void grandma_on_tick(Window *win);
static void grandma_on_click(Window *win, int x, int y);
static void grandma_on_destroy(Window *win);

typedef struct 
{
    int going_left;

    int current_image_index;

    int ticks_for_next_step;
    int ticks_for_next_image;

} Grandma;

int grandma_create(Window *win, int x, int y, int going_left)
{
    if (!win) return 0;

    Grandma *grandma = (Grandma *)malloc(sizeof(Grandma));
    if (!grandma) return 0;

    int image_index = rand() % NUM_GRANDMA_IMAGES;
    BITMAP bm;
    HBITMAP bitmap = hBitmaps[image_index];
    GetObject(bitmap, sizeof(bm), &bm);

    if (!window_create(win, x, y, bm.bmWidth, bm.bmHeight)) { free(grandma); return 0; }
    window_set_tick_callback(win, grandma_on_tick);
    window_set_click_callback(win, grandma_on_click);
    window_set_destroy_callback(win, grandma_on_destroy);
    window_set_bitmap(win, bitmap);

    grandma->going_left = going_left;
    grandma->current_image_index = image_index;
    grandma->ticks_for_next_step = GRANDMA_TICKS_PER_STEP;
    grandma->ticks_for_next_image = GRANDMA_TICKS_PER_IMAGE;

    window_set_data(win, grandma);

    return 1;
}

static void grandma_on_tick(Window *win)
{
    Grandma *grandma = (Grandma *)window_get_data(win);
    if (!grandma) return;

    grandma->ticks_for_next_step--;
    if (grandma->ticks_for_next_step <= 0)
    {
        win->x += grandma->going_left ? -1 : 1;
        grandma->ticks_for_next_step = GRANDMA_TICKS_PER_STEP;

        int window_min_x = win->x;
        int window_max_x = win->x + win->w;

        int collided_x = 0;
        if (grandma->going_left) collided_x = window_max_x <= 0;
        else collided_x = window_min_x >= GetSystemMetrics(SM_CXSCREEN);

        if (collided_x) 
        {
            window_destroy(win);
            return;
        }

        window_move_to(win, win->x, win->y);
    }

    grandma->ticks_for_next_image--;
    if (grandma->ticks_for_next_image <= 0)
    {
        grandma->ticks_for_next_image = GRANDMA_TICKS_PER_IMAGE;
    
        grandma->current_image_index = (grandma->current_image_index + 1) % NUM_GRANDMA_IMAGES;
        window_set_bitmap(win, hBitmaps[grandma->current_image_index]);
    }
}

static void grandma_on_click(Window *win, int x, int y)
{
    (void)x;
    (void)y;
    (void)win;
}

static void grandma_on_destroy(Window *win)
{
    Grandma *grandma = (Grandma *)window_get_data(win);
    if (grandma) free(grandma);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    srand((unsigned int)GetTickCount());

    // Unused parameters (to avoid compiler warnings)
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    if (!init_window_module(hInstance, "Ye old grandmother")) { print_error("Failed to initialize window module"); return 1; };

    int image_witdh = 0;
    for (int i = 0; i < NUM_GRANDMA_IMAGES; i++)
    {
        wchar_t name[20];
        swprintf(name, 20, L"GRANDMA_%03d", i);
        hBitmaps[i] = (HBITMAP) LoadImageW(hInstance, name, IMAGE_BITMAP, 0, 0, 0);
        if (hBitmaps[i] == NULL) { print_error("Failed to load bitmap"); return 2; }

        if (image_witdh == 0)
        {
            BITMAP bm;
            GetObject(hBitmaps[i], sizeof(bm), &bm);
            image_witdh = bm.bmWidth;
        }
    }

    Window grandmas[NUM_GRANDMAS] = {0};
    for (int i = 0; i < NUM_GRANDMAS; i++)
    {
        int going_left = rand() % 2;
        int x = going_left ? GetSystemMetrics(SM_CXSCREEN) : -image_witdh;
        int y = rand() % (GetSystemMetrics(SM_CYSCREEN) - 100 + 1);
        if (!grandma_create(&grandmas[i], x, y, going_left)) { print_error("Failed to create grandma"); return 3; }
    }

    int result = window_run();
    for (int i = 0; i < NUM_GRANDMA_IMAGES; i++)
    {
        DeleteObject(hBitmaps[i]);
    }
    return result;
}