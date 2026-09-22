#include "vulkan_core.h"
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <vector>

namespace {
std::mutex g_mutex;
VkInstance g_instance = VK_NULL_HANDLE;
VkPhysicalDevice g_physical = VK_NULL_HANDLE;
VkDevice g_device = VK_NULL_HANDLE;

void Log(const char* message) noexcept {
    try {
        std::filesystem::create_directories(R"(C:\MaazXGPU\logs)");
        std::ofstream file(R"(C:\MaazXGPU\logs\vulkan.log)", std::ios::app);
        if (file.is_open()) file << message << '\n';
    } catch (...) {}
}
}

namespace MaazXGPU::VulkanCore {
VkResult Initialize() {
    std::lock_guard lock(g_mutex);
    if (g_instance != VK_NULL_HANDLE) return VK_SUCCESS;
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName = "MaazXGPU - Pure Performance Driver for Vostro 3300";
    app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app.pEngineName = "MaazXGPU";
    app.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app.apiVersion = VK_API_VERSION_1_3;
    VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    info.pApplicationInfo = &app;
    VkResult result = vkCreateInstance(&info, nullptr, &g_instance);
    if (result != VK_SUCCESS) { Log("vkCreateInstance failed"); return result; }
    uint32_t count = 0;
    if (vkEnumeratePhysicalDevices(g_instance, &count, nullptr) != VK_SUCCESS || count == 0) {
        Log("No Vulkan physical device found");
        vkDestroyInstance(g_instance, nullptr);
        g_instance = VK_NULL_HANDLE;
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    std::vector<VkPhysicalDevice> devices(count, VK_NULL_HANDLE);
    if (vkEnumeratePhysicalDevices(g_instance, &count, devices.data()) != VK_SUCCESS ||
        count == 0 || devices[0] == VK_NULL_HANDLE) {
        vkDestroyInstance(g_instance, nullptr);
        g_instance = VK_NULL_HANDLE;
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    g_physical = devices[0];
    Log("MaazXGPU Vulkan 1.3 initialized");
    return VK_SUCCESS;
}

void Shutdown() noexcept {
    std::lock_guard lock(g_mutex);
    if (g_device != VK_NULL_HANDLE) { vkDestroyDevice(g_device, nullptr); g_device = VK_NULL_HANDLE; }
    if (g_instance != VK_NULL_HANDLE) { vkDestroyInstance(g_instance, nullptr); g_instance = VK_NULL_HANDLE; }
    g_physical = VK_NULL_HANDLE;
}
VkInstance Instance() noexcept { return g_instance; }
VkPhysicalDevice PhysicalDevice() noexcept { return g_physical; }
VkDevice Device() noexcept { return g_device; }
VkResult CreateBuffer(const VkBufferCreateInfo* info, VkBuffer* buffer) noexcept {
    if (info == nullptr || buffer == nullptr || g_device == VK_NULL_HANDLE) return VK_ERROR_INITIALIZATION_FAILED;
    return vkCreateBuffer(g_device, info, nullptr, buffer);
}
}
