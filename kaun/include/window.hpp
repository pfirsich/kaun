#pragma once

#include <glm/glm.hpp>
#include <SDL.h>
#include <SDL_opengl.h>

#include "signal.hpp"

namespace kaun {
    struct WindowProperties {
        bool fullscreen = false;
        bool fullscreenDesktop = false;
        bool hidden = false;
        bool borderless = false;
        bool resizable = true;
        bool maximized = false;
        bool minimized = false;
        bool vsync = false;
        int depth = 24;
        bool stencil = false;
        bool srgb = false;
        int msaaSamples = 0;
    };

    // returns success
    extern bool setupWindow(const char* title, int width, int height,
                            const WindowProperties& props = WindowProperties());
    extern int getWindowWidth();
    extern int getWindowHeight();
    extern glm::ivec2 getWindowSize();
    extern void update();
    extern void swapBuffers();
    extern void updateAndSwap();
    extern void cleanup();
    extern float getTime();
    // glm::ivec2 getWindowSize()
    // getSdlWindowHandl, getSdlGlContext
    // saveScreenshot

    using CloseSignalType = Signal<void()>;
    extern CloseSignalType closeSignal;

    using ResizeSignalType = Signal<void(int, int)>;
    extern ResizeSignalType resizeSignal;
}
