#pragma once
#include <vulkan/vulkan.h>

namespace MaazXGPU::VulkanCore {
VkResult Initialize();
void Shutdown() noexcept;
VkInstance Instance() noexcept;
VkPhysicalDevice PhysicalDevice() noexcept;
VkDevice Device() noexcept;
VkResult CreateBuffer(const VkBufferCreateInfo* info, VkBuffer* buffer) noexcept;
}
