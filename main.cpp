#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
#include <wininet.h>
#include <shellapi.h>
#include <string>
#include <thread>
#include <atomic>
#include "api.h"
#include "steam.h"
#include "launcher.h"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "shell32.lib")

#define IDC_BTN_PLAY     1001
#define IDC_BTN_WEBSITE  1002
#define IDC_LABEL_STATUS 1003
#define IDC_LABEL_USER   1004

#define CLR_BG       RGB(20,22,26)
#define CLR_PANEL    RGB(29,32,37)
#define CLR_AMBER    RGB(242,183,5)
#define CLR_CHROME   RGB(154,161,170)
#define CLR_WHITE    RGB(244,242,236)
#define CLR_RED      RGB(192,57,43)
#define CLR_GREEN    RGB(76,175,80)

static HWND g_hwnd      = nullptr;
static HWND g_btnPlay   = nullptr;
static HWND g_btnWeb    = nullptr;
static HWND g_lblStatus = nullptr;
static HWND g_lblUser   = nullptr;

static HBRUSH g_bgBrush    = nullptr;
static HBRUSH g_panelBrush = nullptr;
static HFONT  g_fontTitle  = nullptr;
static HFONT  g_fontBody   = nullptr;
static HFONT  g_fontMono   = nullptr;

static std::wstring g_statusMsg;
static COLORREF     g_statusColor = CLR_CHROME;
static std::atomic<bool> g_launching{false};

void SetStatus(const std::wstring& msg, COLORREF color = CLR_CHROME) {
    g_statusMsg   = msg;
    g_statusColor = color;
    if (g_lblStatus) {
        SetWindowTextW(g_lblStatus, msg.c_str());
        InvalidateRect(g_hwnd, nullptr, FALSE);
    }
}

void SetUserLabel(const std::wstring& name) {
    if (g_lblUser)
        SetWindowTextW(g_lblUser, name.empty() ? L"Not logged in" : (L"Logged in as: " + name).c_str());
}

void EnablePlay(bool enabled) {
    if (g_btnPlay) EnableWindow(g_btnPlay, enabled ? TRUE : FALSE);
}

void DoLaunch() {
    EnablePlay(false);
    SetStatus(L"Verifying session...", CLR_CHROME);

    std::wstring user;
    if (!API::CheckSession(user)) {
        SetStatus(L"Not logged in. Register on the Convoy website first.", CLR_RED);
        EnablePlay(true);
        g_launching = false;
        return;
    }
    SetUserLabel(user);
    SetStatus(L"Checking requirements...", CLR_CHROME);

    std::wstring reqErr;
    if (!API::CheckRequirements(reqErr)) {
        SetStatus(reqErr, CLR_RED);
        EnablePlay(true);
        g_launching = false;
        return;
    }
    SetStatus(L"Finding ETS2...", CLR_CHROME);

    std::wstring ets2Path;
    if (!Steam::FindETS2(ets2Path)) {
        SetStatus(L"ETS2 not found. Make sure it is installed via Steam.", CLR_RED);
        EnablePlay(true);
        g_launching = false;
        return;
    }
    SetStatus(L"Launching Euro Truck Simulator 2...", CLR_AMBER);
    Sleep(600);

    if (!Launcher::LaunchETS2(ets2Path)) {
        SetStatus(L"Failed to launch ETS2. Try running as administrator.", CLR_RED);
        EnablePlay(true);
        g_launching = false;
        return;
    }

    SetStatus(L"ETS2 launched! Have a safe journey.", CLR_GREEN);
    Sleep(3000);
    PostMessage(g_hwnd, WM_CLOSE, 0, 0);
    g_launching = false;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wp;
        HWND ctrl = (HWND)lp;
        SetBkMode(hdc, TRANSPARENT);
        if (ctrl == g_lblStatus) {
            SetTextColor(hdc, g_statusColor);
            return (LRESULT)g_panelBrush;
        }
        if (ctrl == g_lblUser) {
            SetTextColor(hdc, CLR_CHROME);
            return (LRESULT)g_bgBrush;
        }
        return (LRESULT)g_bgBrush;
    }
    case WM_CTLCOLORBTN:
        return (LRESULT)g_bgBrush;

    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wp;
        RECT rc; GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, g_bgBrush);
        RECT panel = { 0, 220, rc.right, rc.bottom };
        FillRect(hdc, &panel, g_panelBrush);
        RECT accent = { 0, 0, rc.right, 4 };
        HBRUSH ab = CreateSolidBrush(CLR_AMBER);
        FillRect(hdc, &accent, ab);
        DeleteObject(ab);
        return 1;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        SelectObject(hdc, g_fontTitle);
        SetTextColor(hdc, CLR_WHITE);
        SetBkMode(hdc, TRANSPARENT);
        TextOutW(hdc, 36, 24, L"CONVOY", 6);

        HBRUSH amb = CreateSolidBrush(CLR_AMBER);
        RECT dot = { 152, 34, 160, 42 };
        FillRect(hdc, &dot, amb);
        DeleteObject(amb);

        SelectObject(hdc, g_fontBody);
        SetTextColor(hdc, CLR_CHROME);
        TextOutW(hdc, 36, 76, L"Multiplayer for Euro Truck Simulator 2", 38);

        SelectObject(hdc, g_fontMono);
        SetTextColor(hdc, CLR_CHROME);
        TextOutW(hdc, 36, 100, L"v0.1.0-alpha", 12);

        HPEN pen = CreatePen(PS_SOLID, 1, RGB(42,46,53));
        SelectObject(hdc, pen);
        MoveToEx(hdc, 0, 138, nullptr);
        LineTo(hdc, 600, 138);
        DeleteObject(pen);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_BTN_PLAY:
            if (!g_launching) {
                g_launching = true;
                std::thread(DoLaunch).detach();
            }
            break;
        case IDC_BTN_WEBSITE:
            ShellExecuteW(nullptr, L"open",
                L"https://codepen.io/YOUR_CODEPEN_URL",
                nullptr, nullptr, SW_SHOW);
            break;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

void CreateControls(HWND hwnd) {
    g_fontTitle = CreateFontW(44, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Arial");
    g_fontBody = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    g_fontMono = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Consolas");

    g_lblUser = CreateWindowExW(0, L"STATIC", L"Not logged in",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        36, 152, 400, 22, hwnd, (HMENU)IDC_LABEL_USER, nullptr, nullptr);
    SendMessageW(g_lblUser, WM_SETFONT, (WPARAM)g_fontBody, TRUE);

    g_lblStatus = CreateWindowExW(0, L"STATIC", L"Ready.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        36, 238, 520, 22, hwnd, (HMENU)IDC_LABEL_STATUS, nullptr, nullptr);
    SendMessageW(g_lblStatus, WM_SETFONT, (WPARAM)g_fontMono, TRUE);

    g_btnPlay = CreateWindowExW(0, L"BUTTON", L"PLAY",
        WS_CHILD | WS_VISIBLE | BS_FLAT | BS_PUSHBUTTON,
        36, 278, 200, 52, hwnd, (HMENU)IDC_BTN_PLAY, nullptr, nullptr);
    SendMessageW(g_btnPlay, WM_SETFONT, (WPARAM)g_fontTitle, TRUE);

    g_btnWeb = CreateWindowExW(0, L"BUTTON", L"Open website",
        WS_CHILD | WS_VISIBLE | BS_FLAT | BS_PUSHBUTTON,
        252, 290, 140, 30, hwnd, (HMENU)IDC_BTN_WEBSITE, nullptr, nullptr);
    SendMessageW(g_btnWeb, WM_SETFONT, (WPARAM)g_fontBody, TRUE);

    std::thread([](){
        SetStatus(L"Checking for updates...", CLR_CHROME);
        Sleep(600);
        std::wstring ver;
        if (API::CheckForUpdate(ver)) {
            SetStatus(L"Update available: " + ver + L" — visit website.", CLR_AMBER);
            return;
        }
        std::wstring user;
        if (API::CheckSession(user)) {
            SetUserLabel(user);
            SetStatus(L"Logged in. Click PLAY when ready.", CLR_GREEN);
        } else {
            SetStatus(L"Not logged in. Register on the Convoy website first.", CLR_RED);
            EnablePlay(false);
        }
    }).detach();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    g_bgBrush    = CreateSolidBrush(CLR_BG);
    g_panelBrush = CreateSolidBrush(CLR_PANEL);

    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = L"ConvoyLauncher";
    wc.hbrBackground = g_bgBrush;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon         = LoadIcon(hInstance, IDI_APPLICATION);
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(
        WS_EX_APPWINDOW, L"ConvoyLauncher", L"Convoy Launcher",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        nullptr, nullptr, hInstance, nullptr);

    CreateControls(g_hwnd);
    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    DeleteObject(g_bgBrush);
    DeleteObject(g_panelBrush);
    DeleteObject(g_fontTitle);
    DeleteObject(g_fontBody);
    DeleteObject(g_fontMono);
    return (int)msg.wParam;
}
