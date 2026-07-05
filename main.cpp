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
    if
