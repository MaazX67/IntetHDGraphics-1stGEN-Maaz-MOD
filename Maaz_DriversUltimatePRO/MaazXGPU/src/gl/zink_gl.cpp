#include <windows.h>
#include <GL/gl.h>
#include <atomic>

namespace { std::atomic<bool> g_softwareFallback{true}; HGLRC g_context = nullptr; }

extern "C" __declspec(dllexport) HGLRC WINAPI wglCreateContext(HDC dc) noexcept {
    if (dc == nullptr) return nullptr;
    try {
        HMODULE opengl = LoadLibraryW(L"opengl32.dll");
        if (opengl == nullptr) return nullptr;
        using CreateContextProc = HGLRC (WINAPI *)(HDC);
        const auto createContext = reinterpret_cast<CreateContextProc>(
            GetProcAddress(opengl, "wglCreateContext"));
        if (createContext == nullptr) {
            FreeLibrary(opengl);
            return nullptr;
        }
        HGLRC context = createContext(dc);
        FreeLibrary(opengl);
        if (context != nullptr) { g_context = context; g_softwareFallback = false; return context; }
    } catch (...) {}
    g_softwareFallback = true;
    return nullptr;
}

extern "C" __declspec(dllexport) void WINAPI glDrawArrays(GLenum mode, GLint first, GLsizei count) noexcept {
    if (count <= 0 || g_context == nullptr) return;
    try {
        using DrawArraysProc = void (APIENTRY *)(GLenum, GLint, GLsizei);
        const auto drawArrays = reinterpret_cast<DrawArraysProc>(
            wglGetProcAddress("glDrawArrays"));
        if (drawArrays != nullptr) drawArrays(mode, first, count);
    } catch (...) { g_softwareFallback = true; }
}

extern "C" __declspec(dllexport) GLuint WINAPI glCreateShader(GLenum type) noexcept {
    if (g_context == nullptr) return 0;
    try {
        using CreateShaderProc = GLuint (APIENTRY *)(GLenum);
        const auto createShader = reinterpret_cast<CreateShaderProc>(
            wglGetProcAddress("glCreateShader"));
        return createShader == nullptr ? 0U : createShader(type);
    } catch (...) { g_softwareFallback = true; return 0; }
}
