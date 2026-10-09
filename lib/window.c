#include "window.h"

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE) 
    {
        CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
    }

    Window *win = (Window *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (win == NULL) return DefWindowProc(hwnd, msg, wParam, lParam);

    BITMAP bitmap;

    switch(msg)
    {
        case WM_CREATE:
            // Fired when the window is created
            break;  

        case WM_PAINT:
        {
            // Fired when the window needs to be repainted    
            if (win->on_paint) { win->on_paint(win); break; }

            if (!win->bitmap) break;

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC hdcMem = CreateCompatibleDC(hdc);
            HGDIOBJ oldBitmap = SelectObject(hdcMem, win->bitmap);

            GetObject(win->bitmap, sizeof(bitmap), &bitmap);

            SetStretchBltMode(hdc, COLORONCOLOR);
            StretchBlt(hdc, 0, 0, win->w, win->h, hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);

            SelectObject(hdcMem, oldBitmap);
            DeleteDC(hdcMem);

            EndPaint(hwnd, &ps);
            
            break;
        }
        case WM_TIMER:
            // Fired when a timer event occurs
            if (!win->on_tick) break;

            win->on_tick(win);

            break;

        case WM_LBUTTONDOWN:
            // Fired when the user clicks the left mouse button
            if (!win->on_click) break;

            break; 
        
        case WM_CLOSE:
            // Fired when the window is closed
            DestroyWindow(hwnd);
            break;
        
        case WM_DESTROY:
            // Fired when the window is destroyed
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

static HINSTANCE g_hInstance;
static char g_szClassName[256];

int init_window_module(HINSTANCE hInstance, const char *program_name)
{
	g_hInstance = hInstance;
	snprintf(g_szClassName, sizeof(g_szClassName), "%s", program_name);

	// Register the window class (Style and properties of the window)
	WNDCLASSEX wc;
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(255,0,255));
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc)) return 0;
    return 1;
}

int window_create(Window *win, int x, int y, int w, int h)
{
	if (!win) return 0;

    win->x = x;
	win->y = y;
	win->w = w;
	win->h = h;

	// Create the actual window
    HWND hwnd;
    hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        g_szClassName, "", WS_POPUP,
        x, y, w, h, NULL, NULL, g_hInstance, win);

    if(hwnd == NULL) return 0;
    win->hwnd = hwnd;

    SetLayeredWindowAttributes(hwnd, RGB(255,0,255), 0, LWA_COLORKEY);
    SetTimer(hwnd, 1, 16, NULL);

    ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    UpdateWindow(hwnd);

	return 1;
}

void window_destroy(Window *win)
{
    if (!win) return;

    DestroyWindow(win->hwnd);
}

void window_set_tick_callback(Window *win, void (*on_tick)(Window *win))
{
    if (!win) return;
    win->on_tick = on_tick;
}

void window_set_paint_callback(Window *win, void (*on_paint)(Window *win))
{
    if (!win) return;
    win->on_paint = on_paint;
}

void window_set_click_callback(Window *win, void (*on_click)(Window *win, int mx, int my))
{
    if (!win) return;
    win->on_click = on_click;
}

void window_set_bitmap(Window *win, HBITMAP bitmap)
{
    if (!win) return;

    win->bitmap = bitmap;
    if (win->hwnd) InvalidateRect(win->hwnd, NULL, FALSE);
}

void window_move_to(Window *win, int x, int y)
{
	if (!win) return;
	if (x < 0) x = 0;
	if (y < 0) y = 0;

	win->x = x;
	win->y = y;

    SetWindowPos(win->hwnd, NULL, win->x, win->y, win->w, win->h, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void window_resize(Window *win, int w, int h)
{
	if (!win) return;
	if (w < 0) w = 0;
	if (h < 0) h = 0;

	win->w = w;
	win->h = h;

    SetWindowPos(win->hwnd, NULL, win->x, win->y, win->w, win->h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

int window_run(void)
{
    // The message loop
    MSG Msg;
    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return (int)Msg.wParam;
}