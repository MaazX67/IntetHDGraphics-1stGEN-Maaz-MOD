#include "../core/vulkan_core.h"
#include <windows.h>
#include <d3d12.h>

extern "C" HRESULT WINAPI D3D12CreateDevice(IUnknown* adapter, D3D_FEATURE_LEVEL level, REFIID riid, void** device) noexcept {
    if (device == nullptr) return E_POINTER;
    *device = nullptr;
    if (FAILED(MaazXGPU::VulkanCore::Initialize())) return E_FAIL;
    if (adapter == nullptr && level < D3D_FEATURE_LEVEL_11_0) return E_INVALIDARG;
    (void)riid;
    return E_NOINTERFACE;
}
