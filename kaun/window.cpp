#include <chrono>
#include <cstring>
#include <unordered_map>

#include <SDL_log.h>
#include <glad/glad.h>

#include "log.hpp"
#include "window.hpp"

namespace kaun {
bool sdlInitialized = false;
SDL_Window* sdlWindow = nullptr;
SDL_GLContext sdlGlContext;

CloseSignalType closeSignal;
ResizeSignalType resizeSignal;

float getTime()
{
    static std::chrono::high_resolution_clock::time_point start
        = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> diff = std::chrono::high_resolution_clock::now() - start;
    return diff.count();
}

void sdlLogFunction(void* userdata, int category, SDL_LogPriority priority, const char* message)
{
    LOG_DEBUG("SDL log: %s", message);
}

bool setupWindow(const char* title, int width, int height, const WindowProperties& props)
{
    SDL_LogSetOutputFunction(sdlLogFunction, nullptr);
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

    if (!sdlInitialized) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            return false;
        }
        sdlInitialized = true;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, props.stencil ? 8 : 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, props.depth);

    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, props.srgb ? 1 : 0);

#ifndef NDEBUG
    int contextFlags = 0;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &contextFlags);
    contextFlags |= SDL_GL_CONTEXT_DEBUG_FLAG;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, contextFlags);
#endif

    if (props.msaaSamples > 0) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, props.msaaSamples);
    }

    Uint32 flags = SDL_WINDOW_OPENGL;
    flags |= (props.fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
    flags |= (props.fullscreen && props.fullscreenDesktop ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    flags |= (props.hidden ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN);
    flags |= (props.borderless ? SDL_WINDOW_BORDERLESS : 0);
    flags |= (props.resizable ? SDL_WINDOW_RESIZABLE : 0);
    flags |= (props.maximized ? SDL_WINDOW_MAXIMIZED : 0);
    flags |= (props.minimized ? SDL_WINDOW_MINIMIZED : 0);
    sdlWindow = SDL_CreateWindow(
        title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
    if (sdlWindow == nullptr) {
        LOG_CRITICAL("SDL_CreateWindow failed! - '%s'\n", SDL_GetError());
        return false;
    }

    sdlGlContext = SDL_GL_CreateContext(sdlWindow);
    if (sdlGlContext == 0) {
        LOG_CRITICAL("SDL_GL_CreateContext failed! - '%s'\n", SDL_GetError());
        return false;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        LOG_CRITICAL("Failed to initialize GLAD! - '%s'\n", SDL_GetError());
        return false;
    }

    if (props.vsync) {
        if (SDL_GL_SetSwapInterval(1) < 0) {
            LOG_ERROR("SDL_GL_SetSwapInterval failed! - '%s'\n", SDL_GetError());
        }
    }

    return true;
}

glm::ivec2 getWindowSize()
{
    int w, h;
    SDL_GetWindowSize(sdlWindow, &w, &h);
    return glm::ivec2(w, h);
}

int getWindowWidth()
{
    return getWindowSize().x;
}

int getWindowHeight()
{
    return getWindowSize().y;
}

void update()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
        case SDL_QUIT:
            closeSignal.emit();
            break;
        case SDL_WINDOWEVENT:
            switch (e.window.event) {
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                resizeSignal.emit(e.window.data1, e.window.data2);
                break;
            }
            break;
        }
    }
}

void swapBuffers()
{
    SDL_GL_SwapWindow(sdlWindow);
}

void updateAndSwap()
{
    update();
    swapBuffers();
}

void cleanup()
{
    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();
}
}
