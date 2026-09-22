#include <windows.h>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <mutex>

namespace {
HANDLE g_mapping = nullptr;
void* g_view = nullptr;
std::mutex g_mutex;
size_t g_size = 0;

ULONGLONG TotalPhysicalBytes() noexcept {
    MEMORYSTATUSEX memory{sizeof(memory)};
    return GlobalMemoryStatusEx(&memory) != FALSE ? memory.ullTotalPhys : 0;
}
}

extern "C" __declspec(dllexport) void* AllocVRAM(size_t requested) noexcept {
    if (requested == 0) return nullptr;
    std::lock_guard lock(g_mutex);
    if (g_view != nullptr && requested <= g_size) return g_view;

    const ULONGLONG total = TotalPhysicalBytes();
    if (total == 0) return nullptr;
    const ULONGLONG limit = total <= (4ULL << 30) ? (512ULL << 20) : (2ULL << 30);
    const size_t bytes = requested > limit ? static_cast<size_t>(limit) : requested;

    if (g_view != nullptr) {
        UnmapViewOfFile(g_view);
        g_view = nullptr;
    }
    if (g_mapping != nullptr) {
        CloseHandle(g_mapping);
        g_mapping = nullptr;
    }

    g_mapping = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
                                   static_cast<DWORD>(bytes >> 32), static_cast<DWORD>(bytes),
                                   L"MaazXGPU_VRAM");
    if (g_mapping == nullptr) return nullptr;
    g_view = MapViewOfFile(g_mapping, FILE_MAP_ALL_ACCESS, 0, 0, bytes);
    if (g_view == nullptr) {
        CloseHandle(g_mapping);
        g_mapping = nullptr;
        return nullptr;
    }
    g_size = bytes;
    try {
        const std::filesystem::path log = R"(C:\MaazXGPU\logs\vram.log)";
        std::filesystem::create_directories(log.parent_path());
        std::ofstream(log, std::ios::app) << (total <= (4ULL << 30)
            ? "4GB RAM profile: 512MB VRAM cap\n" : "VRAM pool allocated\n");
    } catch (...) {
    }
    return g_view;
}
