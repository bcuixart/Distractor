#ifndef WINDOW_H
#define WINDOW_H

#include <windows.h>
#include <stdio.h>

typedef struct Window Window;

struct Window 
{
    HWND hwnd;
    int x, y, w, h;

    void (*on_tick)(Window *win);
    void (*on_paint)(Window *win);
    void (*on_click)(Window *win, int mx, int my);

    HBITMAP bitmap;

    void *data;
};

int init_window_module(HINSTANCE hInstance, const char *program_name);

int  window_create(Window *win, int x, int y, int w, int h);
void window_destroy(Window *win);

void window_set_tick_callback(Window *win, void (*on_tick)(Window *win));
void window_set_paint_callback(Window *win, void (*on_paint)(Window *win));
void window_set_click_callback(Window *win, void (*on_click)(Window *win, int mx, int my));

void window_set_bitmap(Window *win, HBITMAP bitmap);

void window_move_to(Window *win, int x, int y);
void window_resize(Window *win, int w, int h);

int  window_run(void);

#endif // WINDOW_H