#pragma once
#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

namespace Steam {

static bool RegRead(HKEY root, const std::wstring& subkey,
                    const std::wstring& value, std::wstring& out) {
    HKEY key;
    if (RegOpenKeyExW(root, subkey.c_str(), 0, KEY_READ, &key) != ERROR_SUCCESS)
        return false;
    wchar_t buf[512];
    DWORD size = sizeof(buf);
    DWORD type = REG_SZ;
    bool ok = (RegQueryValueExW(key, value.c_str(), nullptr, &type,
                                (LPBYTE)buf, &size) == ERROR_SUCCESS);
    RegCloseKey(key);
    if (ok) out = buf;
    return ok;
}

static std::vector<std::wstring> GetSteamLibraries(const std::wstring& steamPath) {
    std::vector<std::wstring> libs;
    libs.push_back(steamPath + L"\\steamapps");

    std::wstring vdfPath = steamPath + L"\\steamapps\\libraryfolders.vdf";
    std::ifstream file(vdfPath);
    if (!file) return libs;

    std::string line;
    while (std::getline(file, line)) {
        size_t pathPos = line.find("\"path\"");
        if (pathPos == std::string::npos) continue;
        size_t q1 = line.find('"', pathPos + 6);
        if (q1 == std::string::npos) continue;
        size_t q2 = line.find('"', q1 + 1);
        if (q2 == std::string::npos) continue;
        std::string p = line.substr(q1 + 1, q2 - q1 - 1);
        std::string cleaned;
        for (size_t i = 0; i < p.size(); i++) {
            if (p[i] == '\\' && i + 1 < p.size() && p[i+1] == '\\') {
                cleaned += '\\'; i++;
            } else cleaned += p[i];
        }
        std::wstring wp(cleaned.begin(), cleaned.end());
        libs.push_back(wp + L"\\steamapps");
    }
    return libs;
}

inline bool FindETS2(std::wstring& exePath) {
    std::wstring steamPath;
    if (!RegRead(HKEY_LOCAL_MACHINE,
                 L"SOFTWARE\\WOW6432Node\\Valve\\Steam",
                 L"InstallPath", steamPath)) {
        if (!RegRead(HKEY_LOCAL_MACHINE,
                     L"SOFTWARE\\Valve\\Steam",
                     L"InstallPath", steamPath)) {
            if (!RegRead(HKEY_CURRENT_USER,
                         L"SOFTWARE\\Valve\\Steam",
                         L"SteamPath", steamPath)) {
                return false;
            }
            std::replace(steamPath.begin(), steamPath.end(), L'/', L'\\');
        }
    }

    auto libraries = GetSteamLibraries(steamPath);
    for (const auto& lib : libraries) {
        std::wstring manifest = lib + L"\\appmanifest_227300.acf";
        if (GetFileAttributesW(manifest.c_str()) == INVALID_FILE_ATTRIBUTES)
            continue;
        std::wstring candidate = lib + L"\\common\\Euro Truck Simulator 2\\bin\\win_x64\\eurotrucks2.exe";
        if (GetFileAttributesW(candidate.c_str()) != INVALID_FILE_ATTRIBUTES) {
            exePath = candidate;
            return true;
        }
        candidate = lib + L"\\common\\Euro Truck Simulator 2\\bin\\win_x86\\eurotrucks2.exe";
        if (GetFileAttributesW(candidate.c_str()) != INVALID_FILE_ATTRIBUTES) {
            exePath = candidate;
            return true;
        }
    }
    return false;
}

} // namespace Steam
