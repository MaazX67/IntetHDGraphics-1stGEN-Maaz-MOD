#include "performance_manager.h"

#include <tlhelp32.h>
#include <algorithm>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <string>

namespace {
bool g_fpsBoost = false;

void Log(const char* message) noexcept {
    try {
        const std::filesystem::path directory = R"(C:\MaazXGPU\logs)";
        std::filesystem::create_directories(directory);
        std::ofstream file(directory / "performance.log", std::ios::app);
        if (file.is_open()) file << message << '\n';
    } catch (...) {
    }
}

bool ProcessPath(DWORD pid, std::wstring& path) noexcept {
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (process == nullptr) return false;
    wchar_t buffer[MAX_PATH]{};
    DWORD length = MAX_PATH;
    const bool result = QueryFullProcessImageNameW(process, 0, buffer, &length) != FALSE;
    if (result) path.assign(buffer, length);
    CloseHandle(process);
    return result;
}

bool IsMinecraftPath(const std::wstring& path) noexcept {
    std::wstring lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](wchar_t value) {
        return static_cast<wchar_t>(std::towlower(value));
    });
    return lower.find(L"javaw.exe") != std::wstring::npos &&
           (lower.find(L"minecraft") != std::wstring::npos ||
            lower.find(L".minecraft") != std::wstring::npos);
}
}

namespace MaazXGPU::Perf {

bool IsFpsBoostEnabled() noexcept { return g_fpsBoost; }

bool SetFpsBoostEnabled(bool enabled) noexcept {
    g_fpsBoost = enabled;
    Log(enabled ? "FPS boost enabled" : "FPS boost disabled");
    return true;
}

bool ApplyHighPerformanceToMinecraft() noexcept {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    bool changed = false;
    for (BOOL more = Process32FirstW(snapshot, &entry); more != FALSE;
         more = Process32NextW(snapshot, &entry)) {
        if (_wcsicmp(entry.szExeFile, L"javaw.exe") != 0) continue;
        std::wstring path;
        if (!ProcessPath(entry.th32ProcessID, path) || !IsMinecraftPath(path)) continue;
        HANDLE process = OpenProcess(PROCESS_SET_INFORMATION, FALSE, entry.th32ProcessID);
        if (process != nullptr) {
            changed = SetPriorityClass(process, HIGH_PRIORITY_CLASS) != FALSE || changed;
            CloseHandle(process);
        }
    }
    CloseHandle(snapshot);
    if (changed) Log("HIGH_PRIORITY_CLASS applied to Minecraft javaw.exe");
    return changed;
}

bool AutoBalanceLowMemorySystem() noexcept {
    MEMORYSTATUSEX memory{sizeof(memory)};
    if (!GlobalMemoryStatusEx(&memory)) return false;
    const ULONGLONG totalGb = memory.ullTotalPhys / (1024ULL * 1024ULL * 1024ULL);
    if (totalGb > 4) {
        Log("Memory balancer checked: system has more than 4GB RAM");
        return false;
    }
    Log("4GB-or-less system detected: conservative RAM/VRAM limits enabled");
    return true;
}
}
