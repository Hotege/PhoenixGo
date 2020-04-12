#include "window.h"
#include <iostream> // TODO: remember to remove
#include <windowsx.h>
#include "../resource.h"
#include "../../../common/go_state.h"

#define proc_implement_begin(type, f, handle) \
type CALLBACK f(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) \
{
#define proc_implement_end() \
}
#define switch_begin(arg) \
switch(arg) \
{
#define s_begin(arg) \
case arg: \
{
#define s_end() \
} \
    break;
#define switch_end() \
}

namespace yw
{
    typedef struct _img
    {
        HDC dc;
        HBITMAP bitmap;
        BITMAP bmp;
    } img;
    const TCHAR yw_window_class[] = L"yao-win-window";
    const TCHAR yw_window_title[] = L"Yao";
    const int yw_window_width = 320;
    const int yw_window_height = 320;
    const int yw_board_size = 320;
    const int yw_split_size = 16;
    HINSTANCE instance = nullptr;
    HFONT text_font = nullptr;
    img board_img, black_img, white_img, select_img;
    HDC window_dc, memory_dc;
    HBITMAP memory_bitmap;
    int down_x = -1, down_y = -1;
    GoState *state = nullptr;
    void load_img(img *e, const int &id)
    {
        e->dc = CreateCompatibleDC(nullptr);
        e->bitmap = (HBITMAP)LoadImage(instance, MAKEINTRESOURCE(id), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
        SelectObject(e->dc, e->bitmap);
        GetObject(e->bitmap, sizeof(BITMAP), &e->bmp);
    }
    void release_img(img *e)
    {
        DeleteObject(e->bitmap);
        DeleteObject(e->dc);
    }
    void adjust_window(HWND hWnd)
    {
        const GoCoordId sz = GoComm::BORDER_SIZE;
        DWORD nw = yw_window_width, nh = yw_window_height;
        nw = (sz + 1) * yw_split_size;
        nh = (sz + 1) * yw_split_size;
        RECT rtWnd, rtCli;
        GetWindowRect(hWnd, &rtWnd);
        GetClientRect(hWnd, &rtCli);
        DWORD tw = rtWnd.right - rtWnd.left - (rtCli.right - rtCli.left);
        DWORD th = rtWnd.bottom - rtWnd.top - (rtCli.bottom - rtCli.top);
        nw += tw;
        nh += th;
        DWORD nx = GetSystemMetrics(SM_CXSCREEN) / 2 - nw / 2;
        DWORD ny = GetSystemMetrics(SM_CYSCREEN) / 2 - nh / 2;
        SetWindowPos(hWnd, nullptr, nx, ny, nw, nh, SWP_NOZORDER);
    }
    void draw(HWND hWnd, BOOL use_select, int cx, int cy)
    {
        RECT rt;
        GetClientRect(hWnd, &rt);
        FillRect(memory_dc, &rt, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
        const GoCoordId sz = GoComm::BORDER_SIZE;
        StretchBlt(
            memory_dc, 0, 0, (sz + 1) * yw_split_size, (sz + 1) * yw_split_size,
            board_img.dc, 0, 0, board_img.bmp.bmWidth, board_img.bmp.bmHeight, SRCCOPY
        );
        for (auto i = 0; i < sz; i++)
        {
            wchar_t h_coord[2];
            wsprintf(h_coord, L"%2d", sz - i);
            SetBkMode(memory_dc, TRANSPARENT);
            wchar_t v_coord = L'A' + i + (i > 7 ? 1 : 0);
            TextOut(memory_dc, (i + 1) * yw_split_size - yw_split_size / 5, 0, &v_coord, 1);
            TextOut(memory_dc, (i + 1) * yw_split_size - yw_split_size / 5, sz * yw_split_size + 2, &v_coord, 1);
            TextOut(memory_dc, 0, (i + 1) * yw_split_size - yw_split_size / 3, h_coord, wcslen(h_coord));
            TextOut(memory_dc, sz * yw_split_size + 2, (i + 1) * yw_split_size - yw_split_size / 3, h_coord, wcslen(h_coord));
            MoveToEx(
                memory_dc,
                yw_split_size,
                (i + 1) * yw_split_size,
                nullptr
            );
            LineTo(
                memory_dc,
                sz * yw_split_size + (i == sz - 1 ? 1 : 0),
                (i + 1) * yw_split_size
            );
            MoveToEx(
                memory_dc,
                (i + 1) * yw_split_size,
                yw_split_size,
                nullptr
            );
            LineTo(
                memory_dc,
                (i + 1) * yw_split_size,
                sz * yw_split_size + (i == sz - 1 ? 1 : 0)
            );
        }
        for (auto y = 0; y < sz; y++)
            for (auto x = 0; x < sz; x++)
                if (
                    (x == 3 && y == 3) || (x == 3 && y == sz / 2) || (x == 3 && y == sz - 4) ||
                    (x == sz / 2 && y == 3) || (x == sz / 2 && y == sz / 2) || (x == sz / 2 && y == sz - 4) ||
                    (x == sz - 4 && y == 3) || (x == sz - 4 && y == sz / 2) || (x == sz - 4 && y == sz - 4)
                )
                {
                    SetPixel(
                        memory_dc,
                        (x + 1) * yw_split_size - 1,
                        (y + 1) * yw_split_size - 1,
                        RGB(0, 0, 0)
                    );
                    SetPixel(
                        memory_dc,
                        (x + 1) * yw_split_size + 1,
                        (y + 1) * yw_split_size - 1,
                        RGB(0, 0, 0)
                    );
                    SetPixel(
                        memory_dc,
                        (x + 1) * yw_split_size - 1,
                        (y + 1) * yw_split_size + 1,
                        RGB(0, 0, 0)
                    );
                    SetPixel(
                        memory_dc,
                        (x + 1) * yw_split_size + 1,
                        (y + 1) * yw_split_size + 1,
                        RGB(0, 0, 0)
                    );
                }
        HMENU hMenu = GetMenu(hWnd);
        UINT moves_state = GetMenuState(hMenu, IDM_MOVES, MF_UNCHECKED | MF_DISABLED);
        GoCoordId last_move = state->GetLastMove();
        const GoStoneColor *board_data = state->GetBoard();
        GoCoordId sx, sy;
        FOR_EACHCOORD(i)
        {
            sx = i / sz;
            sy = i % sz;
            if (board_data[i] == GoComm::BLACK)
                TransparentBlt(
                    memory_dc,
                    (sx + 1) * yw_split_size - yw_split_size / 2, (sy + 1) * yw_split_size - yw_split_size / 2,
                    yw_split_size, yw_split_size,
                    black_img.dc,
                    0, 0, black_img.bmp.bmWidth, black_img.bmp.bmHeight,
                    RGB(255, 0, 255)
                );
            else if (board_data[i] == GoComm::WHITE)
                TransparentBlt(
                    memory_dc,
                    (sx + 1) * yw_split_size - yw_split_size / 2, (sy + 1) * yw_split_size - yw_split_size / 2,
                    yw_split_size, yw_split_size,
                    white_img.dc,
                    0, 0, white_img.bmp.bmWidth, white_img.bmp.bmHeight,
                    RGB(255, 0, 255)
                );
            if (!moves_state)
            {
            }
            else
            {
                if (last_move != GoComm::COORD_UNSET && last_move / sz == sx && last_move % sz == sy)
                {
                    SetPixel(
                        memory_dc,
                        (sx + 1) * yw_split_size - 1,
                        (sy + 1) * yw_split_size - 1,
                        RGB(255, 0, 0)
                    );
                    SetPixel(
                        memory_dc,
                        (sx + 1) * yw_split_size + 1,
                        (sy + 1) * yw_split_size - 1,
                        RGB(255, 0, 0)
                    );
                    SetPixel(
                        memory_dc,
                        (sx + 1) * yw_split_size - 1,
                        (sy + 1) * yw_split_size + 1,
                        RGB(255, 0, 0)
                    );
                    SetPixel(
                        memory_dc,
                        (sx + 1) * yw_split_size + 1,
                        (sy + 1) * yw_split_size + 1,
                        RGB(255, 0, 0)
                    );
                    SetPixel(
                        memory_dc,
                        (sx + 1) * yw_split_size - 1,
                        (sy + 1) * yw_split_size,
                        RGB(255, 0, 0)
                    );
                    SetPixel(
                        memory_dc,
                        (sx + 1) * yw_split_size,
                        (sy + 1) * yw_split_size - 1,
                        RGB(255, 0, 0)
                    );
                    SetPixel(
                        memory_dc,
                        (sx + 1) * yw_split_size + 1,
                        (sy + 1) * yw_split_size,
                        RGB(255, 0, 0)
                    );
                    SetPixel(
                        memory_dc,
                        (sx + 1) * yw_split_size,
                        (sy + 1) * yw_split_size + 1,
                        RGB(255, 0, 0)
                    );
                    SetPixel(
                        memory_dc,
                        (sx + 1) * yw_split_size,
                        (sy + 1) * yw_split_size,
                        RGB(255, 0, 0)
                    );
                }
            }
        }
        if (use_select)
        TransparentBlt(
            memory_dc,
            (cx + 1) * yw_split_size - yw_split_size / 2, (cy + 1) * yw_split_size - yw_split_size / 2,
            yw_split_size, yw_split_size,
            select_img.dc,
            0, 0, select_img.bmp.bmWidth, select_img.bmp.bmHeight,
            RGB(255, 0, 255)
        );
    }
    proc_implement_begin(INT_PTR, yw_about_proc, hDlg)
        switch_begin(message)
        s_begin(WM_INITDIALOG)
            HWND hParent = (HWND)lParam;
            RECT rtParent, rtDlg;
            GetWindowRect(hParent, &rtParent);
            GetWindowRect(hDlg, &rtDlg);
            int nx = rtParent.left + (rtParent.right - rtParent.left) / 2 - (rtDlg.right - rtDlg.left) / 2;
            int ny = rtParent.top + (rtParent.bottom - rtParent.top) / 2 - (rtDlg.bottom - rtDlg.top) / 2;
            SetWindowPos(hDlg, NULL, nx, ny, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
            return (INT_PTR)TRUE;
        s_end()
        s_begin(WM_COMMAND)
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
        s_end()
        switch_end()
        return (INT_PTR)FALSE;
    proc_implement_end()
    proc_implement_begin(LRESULT, yw_window_proc, hWnd)
        switch_begin(message)
        s_begin(WM_CREATE)
            state = new GoState();
            adjust_window(hWnd);
            text_font = CreateFont(12, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"");
            load_img(&board_img, IDB_BOARD);
            load_img(&black_img, IDB_BLACK);
            load_img(&white_img, IDB_WHITE);
            load_img(&select_img, IDB_SELECT);
            window_dc = GetDC(hWnd);
            memory_dc = CreateCompatibleDC(window_dc);
            memory_bitmap = CreateCompatibleBitmap(window_dc, yw_board_size, yw_board_size);
            SelectObject(memory_dc, memory_bitmap);
            SelectObject(memory_dc, text_font);
            SetStretchBltMode(memory_dc, HALFTONE);
            draw(hWnd, FALSE, 0, 0);
            const GoCoordId sz = GoComm::BORDER_SIZE;
            BitBlt(window_dc, 0, 0, (sz + 1) * yw_split_size, (sz + 1) * yw_split_size, memory_dc, 0, 0, SRCCOPY);
        s_end()
        s_begin(WM_PAINT)
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            const GoCoordId sz = GoComm::BORDER_SIZE;
            BitBlt(window_dc, 0, 0, (sz + 1) * yw_split_size, (sz + 1) * yw_split_size, memory_dc, 0, 0, SRCCOPY);
            EndPaint(hWnd, &ps);
        s_end()
        s_begin(WM_COMMAND)
            switch_begin(LOWORD(wParam))
            s_begin(IDM_MOVES)
                HMENU hMenu = GetMenu(hWnd);
                if (GetMenuState(hMenu, IDM_MOVES, MF_CHECKED))
                    CheckMenuItem(hMenu, IDM_MOVES, MF_UNCHECKED);
                else
                    CheckMenuItem(hMenu, IDM_MOVES, MF_CHECKED);
            s_end()
            s_begin(IDM_PASS)
                state->Move(-1, -1);
            s_end()
            s_begin(IDM_EXIT)
                SendMessage(hWnd, WM_CLOSE, 0, 0);
            s_end()
            s_begin(IDM_ABOUT)
                DialogBoxParam(instance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, yw_about_proc, (LPARAM)hWnd);
            s_end()
            switch_end()
        s_end()
        s_begin(WM_MOUSEMOVE)
            const GoCoordId sz = GoComm::BORDER_SIZE;
            if (sz != 9 && sz != 13 && sz != 19)
                break;
            int x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
            if (x >= yw_split_size / 2 && x < yw_split_size * sz + yw_split_size / 2 && y >= yw_split_size / 2 && y < yw_split_size * sz + yw_split_size / 2)
            {
                int bx = (x - yw_split_size / 2) / yw_split_size;
                int by = (y - yw_split_size / 2) / yw_split_size;
                draw(hWnd, TRUE, bx, by);
                if (sz == 9 || sz == 13 || sz == 19)
                    BitBlt(window_dc, 0, 0, (sz + 1) * yw_split_size, (sz + 1) * yw_split_size, memory_dc, 0, 0, SRCCOPY);
                else
                    BitBlt(window_dc, 0, 0, yw_board_size, yw_board_size, memory_dc, 0, 0, SRCCOPY);
            }
            else
            {
                draw(hWnd, FALSE, 0, 0);
                if (sz == 9 || sz == 13 || sz == 19)
                    BitBlt(window_dc, 0, 0, (sz + 1) * yw_split_size, (sz + 1) * yw_split_size, memory_dc, 0, 0, SRCCOPY);
                else
                    BitBlt(window_dc, 0, 0, yw_board_size, yw_board_size, memory_dc, 0, 0, SRCCOPY);
            }
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.dwHoverTime = 1;
            tme.hwndTrack = hWnd;
            TrackMouseEvent(&tme);
        s_end()
        s_begin(WM_MOUSELEAVE)
            down_x = -1;
            down_y = -1;
        s_end()
        s_begin(WM_LBUTTONDOWN)
            down_x = (GET_X_LPARAM(lParam) - yw_split_size / 2) / yw_split_size;
            down_y = (GET_Y_LPARAM(lParam) - yw_split_size / 2) / yw_split_size;
        s_end()
        s_begin(WM_LBUTTONUP)
            const GoCoordId sz = GoComm::BORDER_SIZE;
            if (sz != 9 && sz != 13 && sz != 19)
            {
                down_x = -1;
                down_y = -1;
                break;
            }
            int bx = (GET_X_LPARAM(lParam) - yw_split_size / 2) / yw_split_size;
            int by = (GET_Y_LPARAM(lParam) - yw_split_size / 2) / yw_split_size;
            if (bx == down_x && by == down_y)
            {
                state->Move(bx, by);
                draw(hWnd, FALSE, 0, 0);
                BitBlt(window_dc, 0, 0, (sz + 1) * yw_split_size, (sz + 1) * yw_split_size, memory_dc, 0, 0, SRCCOPY);
                {
                }
            }
            down_x = -1;
            down_y = -1;
        s_end()
        s_begin(WM_DESTROY)
            DeleteObject(memory_bitmap);
            DeleteObject(memory_dc);
            ReleaseDC(hWnd, window_dc);
            release_img(&select_img);
            release_img(&white_img);
            release_img(&black_img);
            release_img(&board_img);
            DeleteObject(text_font);
            delete state; state = nullptr;
            PostQuitMessage(0);
        s_end()
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        switch_end()
        return 0;
    proc_implement_end()
    ATOM register_window_class(HINSTANCE hInstance)
    {
        instance = hInstance;
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_VREDRAW | CS_HREDRAW;
        wcex.lpfnWndProc = yw_window_proc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
        wcex.lpszMenuName = MAKEINTRESOURCE(IDC_MENU);
        wcex.lpszClassName = yw_window_class;
        wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
        ATOM r = RegisterClassEx(&wcex);
        return r;
    }
    BOOL init_window()
    {
        RECT rt = { 0, 0, yw_window_width, yw_window_height };
        AdjustWindowRect(&rt, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, TRUE);
        DWORD nx = GetSystemMetrics(SM_CXSCREEN) / 2 - (rt.right - rt.left) / 2;
        DWORD ny = GetSystemMetrics(SM_CYSCREEN) / 2 - (rt.bottom - rt.top) / 2;
        HWND handle = CreateWindowEx(
            WS_EX_ACCEPTFILES, yw_window_class, yw_window_title,
            WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            nx, ny, rt.right - rt.left, rt.bottom - rt.top,
            nullptr, nullptr, instance, nullptr
        );
        if (!handle)
            return FALSE;
        ShowWindow(handle, SW_SHOW);
        UpdateWindow(handle);
        return TRUE;
    }
    VOID get_message()
    {
        HACCEL hAccelTable = LoadAccelerators(instance, MAKEINTRESOURCE(IDC_MENU));
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0))
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
    }
} // namespace yw
