#pragma once
#include <windows.h>

namespace MaazXGPU::Perf {
bool IsFpsBoostEnabled() noexcept;
bool SetFpsBoostEnabled(bool enabled) noexcept;
bool ApplyFpsBoostToMinecraft() noexcept;
bool ApplyHighPerformanceToMinecraft() noexcept;
bool ApplyGamePerformanceProfiles() noexcept;
bool AutoBalanceLowMemorySystem() noexcept;
}
