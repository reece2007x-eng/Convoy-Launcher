#pragma once
#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
#include <wininet.h>
#include <string>

#define API_HOST L"web-production-3a22c.up.railway.app"
#define API_PORT 443

namespace API {

static std::string HttpGet(const std::wstring& path) {
    HINTERNET hNet = InternetOpenW(L"ConvoyLauncher/0.1",
        INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    if (!hNet) return "";

    HINTERNET hConn = InternetConnectW(hNet, API_HOST, API_PORT,
        nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConn) { InternetCloseHandle(hNet); return ""; }

    DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE;
    HINTERNET hReq = HttpOpenRequestW(hConn, L"GET", path.c_str(),
        nullptr, nullptr, nullptr, flags, 0);
    if (!hReq) { InternetCloseHandle(hConn); InternetCloseHandle(hNet); return ""; }

    HttpSendRequestW(hReq, nullptr, 0, nullptr, 0);

    std::string result;
    char buf[4096];
    DWORD read = 0;
    while (InternetReadFile(hReq, buf, sizeof(buf) - 1, &read) && read > 0) {
        buf[read] = 0;
        result += buf;
        read = 0;
    }

    InternetCloseHandle(hReq);
    InternetCloseHandle(hConn);
    InternetCloseHandle(hNet);
    return result;
}

static std::string HttpPost(const std::wstring& path, const std::string& body) {
    HINTERNET hNet = InternetOpenW(L"ConvoyLauncher/0.1",
        INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    if (!hNet) return "";

    HINTERNET hConn = InternetConnectW(hNet, API_HOST, API_PORT,
        nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConn) { InternetCloseHandle(hNet); return ""; }

    DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE;
    HINTERNET hReq = HttpOpenRequestW(hConn, L"POST", path.c_str(),
        nullptr, nullptr, nullptr, flags, 0);
    if (!hReq) { InternetCloseHandle(hConn); InternetCloseHandle(hNet); return ""; }

    std::wstring headers = L"Content-Type: application/json\r\n";
    HttpSendRequestW(hReq, headers.c_str(), (DWORD)headers.size(),
        (LPVOID)body.c_str(), (DWORD)body.size());

    std::string result;
    char buf[4096];
    DWORD read = 0;
    while (InternetReadFile(hReq, buf, sizeof(buf) - 1, &read) && read > 0) {
        buf[read] = 0;
        result += buf;
        read = 0;
    }

    InternetCloseHandle(hReq);
    InternetCloseHandle(hConn);
    InternetCloseHandle(hNet);
    return result;
}

static std::wstring JsonGetStr(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return L"";
    pos += search.size();
    size_t end = json.find('"', pos);
    if (end == std::string::npos) return L"";
    std::string val = json.substr(pos, end - pos);
    return std::wstring(val.begin(), val.end());
}

static bool JsonGetBool(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":true";
    return json.find(search) != std::string::npos;
}

inline bool CheckSession(std::wstring& username) {
    std::string resp = HttpGet(L"/api/me");
    if (resp.empty() || resp.find("error") != std::string::npos) return false;
    username = JsonGetStr(resp, "username");
    return !username.empty();
}

inline bool CheckRequirements(std::wstring& errorMsg) {
    std::string resp = HttpGet(L"/api/me");
    if (resp.empty()) {
        errorMsg = L"Could not reach Convoy servers. Check your connection.";
        return false;
    }
    if (!JsonGetBool(resp, "emailVerified")) {
        errorMsg = L"Email not verified. Visit the Convoy website to verify.";
        return false;
    }
    if (!JsonGetBool(resp, "steamOk")) {
        errorMsg = L"Steam playtime not verified. Visit the Convoy website.";
        return false;
    }
    if (!JsonGetBool(resp, "wotOk")) {
        errorMsg = L"World of Trucks not verified. Visit the Convoy website.";
        return false;
    }
    return true;
}

inline bool CheckForUpdate(std::wstring& newVersion) {
    std::string resp = HttpGet(L"/api/version");
    if (resp.empty()) return false;
    std::wstring ver = JsonGetStr(resp, "version");
    if (ver.empty() || ver == L"0.1.0") return false;
    newVersion = ver;
    return true;
}

} // namespace API
