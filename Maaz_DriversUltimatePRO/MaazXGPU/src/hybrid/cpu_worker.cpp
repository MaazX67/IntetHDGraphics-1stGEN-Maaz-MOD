#include <windows.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>
#include <atomic>

namespace { std::mutex mutex; std::condition_variable condition; std::vector<std::jthread> workers; std::vector<std::function<void()>> jobs; std::atomic<bool> stopping{false};
void Worker(std::stop_token token) { for (;;) { std::function<void()> job; { std::unique_lock lock(mutex); condition.wait(lock, [&] { return stopping || token.stop_requested() || !jobs.empty(); }); if ((stopping || token.stop_requested()) && jobs.empty()) return; if (!jobs.empty()) { job = std::move(jobs.back()); jobs.pop_back(); } } if (job) try { job(); } catch (...) {} } }
}
extern "C" __declspec(dllexport) bool StartChunkWorkers() noexcept { std::lock_guard lock(mutex); if (!workers.empty()) return true; stopping = false; try { for (int i = 0; i < 4; ++i) workers.emplace_back(Worker); } catch (...) { stopping = true; workers.clear(); return false; } return true; }
extern "C" __declspec(dllexport) void CullChunks() noexcept { std::lock_guard lock(mutex); if (!stopping) condition.notify_one(); }
extern "C" __declspec(dllexport) void StopChunkWorkers() noexcept { { std::lock_guard lock(mutex); stopping = true; } condition.notify_all(); workers.clear(); { std::lock_guard lock(mutex); jobs.clear(); } }
