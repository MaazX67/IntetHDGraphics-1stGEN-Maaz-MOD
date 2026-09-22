#include <windows.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <algorithm>

namespace { const std::filesystem::path cache = R"(C:\MaazXGPU\Cache\shader.bin)"; constexpr uintmax_t limit = 2ULL * 1024 * 1024 * 1024; }

extern "C" __declspec(dllexport) bool SavePipelineCache(const void* data, size_t size) noexcept {
    if (data == nullptr || size == 0 || size > limit) return false;
    try { std::filesystem::create_directories(cache.parent_path()); std::ofstream out(cache, std::ios::binary | std::ios::trunc); if (!out.is_open()) return false; out.write(static_cast<const char*>(data), static_cast<std::streamsize>(size)); return out.good(); } catch (...) { return false; }
}
extern "C" __declspec(dllexport) size_t LoadPipelineCache(void* data, size_t capacity) noexcept {
    if (data == nullptr || capacity == 0) return 0;
    try { if (!std::filesystem::exists(cache) || std::filesystem::file_size(cache) > limit) return 0; std::ifstream in(cache, std::ios::binary); in.read(static_cast<char*>(data), static_cast<std::streamsize>(capacity)); return static_cast<size_t>(in.gcount()); } catch (...) { return 0; }
}
