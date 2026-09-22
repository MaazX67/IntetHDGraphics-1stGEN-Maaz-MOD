#include <windows.h>
#include <shellapi.h>
#include "../perf/performance_manager.h"

namespace {
constexpr UINT WM_TRAY = WM_APP + 1;
constexpr UINT ID_FPS_BOOST = 1001;
constexpr UINT ID_MINECRAFT = 1002;
constexpr UINT ID_ALL_GAMES = 1003;
constexpr UINT ID_BALANCE = 1004;
constexpr UINT ID_ABOUT = 1005;
constexpr UINT ID_EXIT = 1006;
constexpr UINT AUTO_SCAN_TIMER = 1;

void ShowStatus(HWND owner, const wchar_t* title, const wchar_t* text) noexcept {
    MessageBoxW(owner, text, title, MB_OK | MB_ICONINFORMATION);
}

void ShowMenu(HWND window) noexcept {
    HMENU menu = CreatePopupMenu();
    if (!menu) return;
    const UINT flags = MF_STRING | (MaazXGPU::Perf::IsFpsBoostEnabled() ? MF_CHECKED : MF_UNCHECKED);
    AppendMenuW(menu, flags, ID_FPS_BOOST, L"FPS Boost (Minecraft/Roblox/DX12)");
    AppendMenuW(menu, MF_STRING, ID_MINECRAFT, L"Boost Minecraft now");
    AppendMenuW(menu, MF_STRING, ID_ALL_GAMES, L"Boost supported games now");
    AppendMenuW(menu, MF_STRING, ID_BALANCE, L"Balance RAM/VRAM (4GB mode)");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, ID_ABOUT, L"About");
    AppendMenuW(menu, MF_STRING, ID_EXIT, L"Exit");
    POINT point{};
    GetCursorPos(&point);
    SetForegroundWindow(window);
    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, window, nullptr);
    DestroyMenu(menu);
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) noexcept {
    if (message == WM_TRAY && lparam == WM_RBUTTONUP) { ShowMenu(window); return 0; }
    if (message == WM_TIMER && wparam == AUTO_SCAN_TIMER) {
        if (MaazXGPU::Perf::IsFpsBoostEnabled()) MaazXGPU::Perf::ApplyGamePerformanceProfiles();
        return 0;
    }
    if (message == WM_COMMAND) {
        switch (LOWORD(wparam)) {
        case ID_FPS_BOOST:
            MaazXGPU::Perf::SetFpsBoostEnabled(!MaazXGPU::Perf::IsFpsBoostEnabled());
            return 0;
        case ID_MINECRAFT:
            ShowStatus(window, L"MaazXGPU", MaazXGPU::Perf::ApplyHighPerformanceToMinecraft()
                ? L"Minecraft Java priority profile applied."
                : L"Minecraft javaw.exe was not found or could not be updated.");
            return 0;
        case ID_ALL_GAMES:
            ShowStatus(window, L"MaazXGPU", MaazXGPU::Perf::ApplyGamePerformanceProfiles()
                ? L"Supported game priority profile applied."
                : L"No supported game was found or could be updated.");
            return 0;
        case ID_BALANCE:
            ShowStatus(window, L"MaazXGPU", MaazXGPU::Perf::AutoBalanceLowMemorySystem()
                ? L"4GB memory profile enabled; conservative VRAM allocation is active."
                : L"The 4GB profile was not needed.");
            return 0;
        case ID_ABOUT:
            ShowStatus(window, L"MaazXGPU", L"Safe user-mode performance layer. It cannot create hardware FPS or replace the Intel driver.");
            return 0;
        case ID_EXIT:
            KillTimer(window, AUTO_SCAN_TIMER); PostQuitMessage(0); return 0;
        default: break;
        }
    }
    return DefWindowProcW(window, message, wparam, lparam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
    WNDCLASSW wc{};
    wc.hInstance = instance;
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = L"MaazXGPUTray";
    if (!RegisterClassW(&wc)) return 1;
    HWND window = CreateWindowExW(0, wc.lpszClassName, L"MaazXGPU Tray", WS_OVERLAPPED,
                                  0, 0, 0, 0, nullptr, nullptr, instance, nullptr);
    if (!window) return 1;
    NOTIFYICONDATAW icon{};
    icon.cbSize = sizeof(icon); icon.hWnd = window; icon.uID = 1;
    icon.uFlags = NIF_MESSAGE | NIF_TIP; icon.uCallbackMessage = WM_TRAY;
    wcscpy_s(icon.szTip, L"MaazXGPU performance layer");
    if (!Shell_NotifyIconW(NIM_ADD, &icon)) return 1;
    MaazXGPU::Perf::AutoBalanceLowMemorySystem();
    SetTimer(window, AUTO_SCAN_TIMER, 5000, nullptr);
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) { TranslateMessage(&message); DispatchMessageW(&message); }
    Shell_NotifyIconW(NIM_DELETE, &icon);
    return 0;
}
