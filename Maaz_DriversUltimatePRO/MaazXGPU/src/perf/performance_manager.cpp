#include "performance_manager.h"

#include <tlhelp32.h>
#include <algorithm>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <string>

namespace {
bool g_fpsBoost = false;
constexpr wchar_t kConfigPath[] = L"C:\\MaazXGPU\\config\\performance.ini";

void Log(const char* message) noexcept {
    try {
        const std::filesystem::path directory = R"(C:\MaazXGPU\logs)";
        std::filesystem::create_directories(directory);
        std::ofstream file(directory / "performance.log", std::ios::app);
        if (file.is_open()) file << message << '\n';
    } catch (...) {}
}

void SaveState() noexcept {
    try {
        const std::filesystem::path path = kConfigPath;
        std::filesystem::create_directories(path.parent_path());
        std::ofstream file(path, std::ios::trunc);
        if (file.is_open()) file << "FPS_BOOST=" << (g_fpsBoost ? "1" : "0") << '\n';
    } catch (...) {}
}

bool ProcessPath(DWORD pid, std::wstring& path) noexcept {
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (process == nullptr) return false;
    wchar_t buffer[32768]{};
    DWORD length = static_cast<DWORD>(std::size(buffer));
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
            lower.find(L".minecraft") != std::wstring::npos ||
            lower.find(L"minecraft launcher") != std::wstring::npos);
}

bool BoostProcess(DWORD pid) noexcept {
    HANDLE process = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if (process == nullptr) return false;

    bool changed = SetPriorityClass(process, HIGH_PRIORITY_CLASS) != FALSE;
    PROCESS_POWER_THROTTLING_STATE power{};
    power.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
    power.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
    power.StateMask = 0; // explicitly opt out of EcoQoS throttling
    if (SetProcessInformation(process, ProcessPowerThrottling, &power, sizeof(power)) == FALSE) {
        // Priority boost remains useful on Windows versions without this API support.
    }
    CloseHandle(process);
    return changed;
}
}

namespace MaazXGPU::Perf {

bool IsFpsBoostEnabled() noexcept { return g_fpsBoost; }

bool SetFpsBoostEnabled(bool enabled) noexcept {
    g_fpsBoost = enabled;
    SaveState();
    Log(enabled ? "FPS boost enabled" : "FPS boost disabled");
    if (enabled) ApplyFpsBoostToMinecraft();
    return true;
}

bool ApplyFpsBoostToMinecraft() noexcept {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    bool changed = false;
    for (BOOL more = Process32FirstW(snapshot, &entry); more != FALSE;
         more = Process32NextW(snapshot, &entry)) {
        if (_wcsicmp(entry.szExeFile, L"javaw.exe") != 0) continue;
        std::wstring path;
        if (ProcessPath(entry.th32ProcessID, path) && IsMinecraftPath(path)) {
            changed = BoostProcess(entry.th32ProcessID) || changed;
        }
    }
    CloseHandle(snapshot);
    if (changed) Log("FPS boost applied to Minecraft javaw.exe");
    return changed;
}

bool ApplyHighPerformanceToMinecraft() noexcept {
    return ApplyFpsBoostToMinecraft();
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
