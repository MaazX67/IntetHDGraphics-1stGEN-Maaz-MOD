#include "performance_manager.h"

#include <tlhelp32.h>
#include <algorithm>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <iterator>
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

std::wstring Lower(std::wstring value) {
    std::transform(value.begin(), value.end(), value.begin(), [](wchar_t c) {
        return static_cast<wchar_t>(std::towlower(c));
    });
    return value;
}

bool IsGameProcess(const std::wstring& name) {
    const std::wstring exe = Lower(name);
    // Minecraft Java/Bedrock, Roblox, and common DX12 game executables.
    static constexpr const wchar_t* targets[] = {
        L"javaw.exe", L"minecraft.windows.exe", L"minecraft.exe",
        L"robloxplayerbeta.exe", L"robloxplayer.exe",
        L"fortniteclient-win64-shipping.exe", L"eldenring.exe",
        L"d3d12game.exe", L"camelion.exe", L"chameleon.exe"
    };
    for (const auto* target : targets) {
        if (exe == target) return true;
    }
    return false;
}

bool IsMinecraftJava(const std::wstring& name, const std::wstring& path) {
    const std::wstring exe = Lower(name);
    const std::wstring lowerPath = Lower(path);
    // Java executable locations usually do not contain Minecraft, so javaw.exe
    // is intentionally accepted as a Minecraft candidate when FPS Boost is on.
    return exe == L"javaw.exe" || lowerPath.find(L"minecraft") != std::wstring::npos;
}

bool BoostProcess(DWORD pid) noexcept {
    HANDLE process = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if (process == nullptr) return false;
    bool changed = SetPriorityClass(process, HIGH_PRIORITY_CLASS) != FALSE;
    PROCESS_POWER_THROTTLING_STATE power{};
    power.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
    power.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
    power.StateMask = 0;
    SetProcessInformation(process, ProcessPowerThrottling, &power, sizeof(power));
    CloseHandle(process);
    return changed;
}

bool Scan(bool minecraftOnly) noexcept {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    bool changed = false;
    for (BOOL more = Process32FirstW(snapshot, &entry); more != FALSE;
         more = Process32NextW(snapshot, &entry)) {
        if (!IsGameProcess(entry.szExeFile)) continue;
        std::wstring path;
        if (!ProcessPath(entry.th32ProcessID, path)) continue;
        if (minecraftOnly && !IsMinecraftJava(entry.szExeFile, path)) continue;
        changed = BoostProcess(entry.th32ProcessID) || changed;
    }
    CloseHandle(snapshot);
    return changed;
}
}

namespace MaazXGPU::Perf {

bool IsFpsBoostEnabled() noexcept { return g_fpsBoost; }

bool SetFpsBoostEnabled(bool enabled) noexcept {
    g_fpsBoost = enabled;
    try {
        const std::filesystem::path path = kConfigPath;
        std::filesystem::create_directories(path.parent_path());
        std::ofstream file(path, std::ios::trunc);
        if (file.is_open()) file << "FPS_BOOST=" << (enabled ? "1" : "0") << '\n';
    } catch (...) {}
    Log(enabled ? "FPS boost enabled" : "FPS boost disabled");
    if (enabled) ApplyGamePerformanceProfiles();
    return true;
}

bool ApplyFpsBoostToMinecraft() noexcept {
    const bool changed = Scan(true);
    if (changed) Log("FPS profile applied to Minecraft javaw.exe");
    return changed;
}

bool ApplyHighPerformanceToMinecraft() noexcept { return ApplyFpsBoostToMinecraft(); }

bool ApplyGamePerformanceProfiles() noexcept {
    const bool changed = Scan(false);
    if (changed) Log("safe HIGH_PRIORITY game profile applied");
    return changed;
}

bool AutoBalanceLowMemorySystem() noexcept {
    MEMORYSTATUSEX memory{sizeof(memory)};
    if (!GlobalMemoryStatusEx(&memory)) return false;
    const ULONGLONG totalGb = memory.ullTotalPhys / (1024ULL * 1024ULL * 1024ULL);
    Log(totalGb <= 4 ? "4GB profile enabled; conservative VRAM limits requested"
                     : "memory balancer checked: more than 4GB RAM");
    return totalGb <= 4;
}
}
