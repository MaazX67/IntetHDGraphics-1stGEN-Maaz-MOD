#include <windows.h>
#include <cstddef>
#include <mutex>
#include <filesystem>
#include <fstream>

namespace { HANDLE g_mapping = nullptr; void* g_view = nullptr; std::mutex g_mutex; size_t g_size = 0; }

extern "C" __declspec(dllexport) void* AllocVRAM(size_t requested) noexcept {
    if (requested == 0) return nullptr;
    std::lock_guard lock(g_mutex);
    if (g_view != nullptr && requested <= g_size) return g_view;
    MEMORYSTATUSEX memory{sizeof(memory)};
    if (!GlobalMemoryStatusEx(&memory)) return nullptr;
    const ULONGLONG pool = memory.ullTotalPhys < (6ULL << 30) ? (1ULL << 30) : (2ULL << 30);
    const size_t bytes = requested > pool ? static_cast<size_t>(pool) : requested;
    if (g_view != nullptr) { UnmapViewOfFile(g_view); g_view = nullptr; }
    if (g_mapping != nullptr) { CloseHandle(g_mapping); g_mapping = nullptr; }
    g_mapping = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
                                   static_cast<DWORD>(bytes >> 32), static_cast<DWORD>(bytes), L"MaazXGPU_VRAM");
    if (g_mapping == nullptr) return nullptr;
    g_view = MapViewOfFile(g_mapping, FILE_MAP_ALL_ACCESS, 0, 0, bytes);
    if (g_view == nullptr) { CloseHandle(g_mapping); g_mapping = nullptr; return nullptr; }
    g_size = bytes;
    try { std::filesystem::create_directories(R"(C:\MaazXGPU\logs)"); std::ofstream(R"(C:\MaazXGPU\logs\vram.log)", std::ios::app) << "VRAM pool allocated\n"; } catch (...) {}
    return g_view;
}
