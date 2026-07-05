#pragma once
#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
#include <string>

namespace Launcher {

inline bool LaunchETS2(const std::wstring& exePath) {
    std::wstring workDir = exePath.substr(0, exePath.rfind(L'\\'));
    std::wstring args = L"\"" + exePath + L"\" -noworkshop -nointro";

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    BOOL ok = CreateProcessW(
        exePath.c_str(),
        &args[0],
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        workDir.c_str(),
        &si,
        &pi
    );

    if (ok) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    return ok == TRUE;
}

} // namespace Launcher
