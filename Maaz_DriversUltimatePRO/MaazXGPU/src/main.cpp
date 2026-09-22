#include <windows.h>
#include <filesystem>
#include "core/vulkan_core.h"

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) noexcept { (void)module; (void)reserved; if (reason == DLL_PROCESS_ATTACH) { DisableThreadLibraryCalls(module); try { std::filesystem::create_directories(R"(C:\MaazXGPU\logs)"); std::filesystem::create_directories(R"(C:\MaazXGPU\Cache)"); } catch (...) { return FALSE; } } else if (reason == DLL_PROCESS_DETACH) { MaazXGPU::VulkanCore::Shutdown(); } return TRUE; }
