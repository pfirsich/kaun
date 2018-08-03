#include <chrono>
#include <cstring>
#include <unordered_map>

#include <glad/glad.h>
#include <SDL_log.h>

#include "log.hpp"
#include "window.hpp"

namespace kaun {
    bool sdlInitialized = false;
    SDL_Window* sdlWindow = nullptr;
    SDL_GLContext sdlGlContext;

    CloseSignalType closeSignal;
    ResizeSignalType resizeSignal;

    void checkGLError() {
        GLenum err = glGetError();
        if(err != GL_NO_ERROR) {
            std::string text("Unknown error");
            switch(err) {
                case GL_INVALID_ENUM:
                    text = "GL_INVALID_ENUM"; break;
                case GL_INVALID_VALUE:
                    text = "GL_INVALID_VALUE"; break;
                case GL_INVALID_OPERATION:
                    text = "GL_INVALID_OPERATION"; break;
                case GL_INVALID_FRAMEBUFFER_OPERATION:
                    text = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
                case GL_OUT_OF_MEMORY:
                    text = "GL_OUT_OF_MEMORY"; break;
            }
            LOG_WARNING("GL Error!: 0x%X - %s\n", err, text.c_str());
        }
    }

    float getTime() {
        static std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> diff = std::chrono::high_resolution_clock::now() - start;
        return diff.count();
    }

    std::unordered_map<GLenum, const char*> debugSourceName = {
        {GL_DEBUG_SOURCE_API, "api"},
        {GL_DEBUG_SOURCE_SHADER_COMPILER, "shader_compiler"},
        {GL_DEBUG_SOURCE_WINDOW_SYSTEM, "window_system"},
        {GL_DEBUG_SOURCE_THIRD_PARTY, "third_party"},
        {GL_DEBUG_SOURCE_APPLICATION, "application"},
        {GL_DEBUG_SOURCE_OTHER, "other"}
    };

    std::unordered_map<GLenum, const char*> debugTypeName = {
        {GL_DEBUG_TYPE_ERROR,"error"},
        {GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR,"deprecated_behavior"},
        {GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,"undefined_behavior"},
        {GL_DEBUG_TYPE_PERFORMANCE,"performance"},
        {GL_DEBUG_TYPE_PORTABILITY,"portability"},
        {GL_DEBUG_TYPE_OTHER,"other"},
        {GL_DEBUG_TYPE_MARKER,"marker"},
        {GL_DEBUG_TYPE_PUSH_GROUP,"push_group"},
        {GL_DEBUG_TYPE_POP_GROUP,"pop_group"}
    };

    std::unordered_map<GLenum, const char*> debugSeverityName = {
        {GL_DEBUG_SEVERITY_HIGH, "high"},
        {GL_DEBUG_SEVERITY_MEDIUM, "medium"},
        {GL_DEBUG_SEVERITY_LOW, "low"},
        {GL_DEBUG_SEVERITY_NOTIFICATION, "notification"}
    };

    void __stdcall debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
        if(severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
        LOG_DEBUG("GL Debug message - source: %s, type: %s, severity: %s, message: %s", debugSourceName[source], debugTypeName[type], debugSeverityName[severity], message);
    }

    void sdlLogFunction(void* userdata, int category, SDL_LogPriority priority, const char* message) {
        LOG_DEBUG("SDL log: %s", message);
    }

    bool setupWindow(const char* title, int width, int height, const WindowProperties& props) {
        SDL_LogSetOutputFunction(sdlLogFunction, nullptr);
        SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

        if(!sdlInitialized) {
            if(SDL_Init(SDL_INIT_VIDEO) < 0) {
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

        if(props.msaaSamples > 0) {
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
        sdlWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
        if(sdlWindow == nullptr) {
            LOG_CRITICAL("SDL_CreateWindow failed! - '%s'\n", SDL_GetError());
            return false;
        }

        sdlGlContext = SDL_GL_CreateContext(sdlWindow);
        if(sdlGlContext == 0) {
            LOG_CRITICAL("SDL_GL_CreateContext failed! - '%s'\n", SDL_GetError());
            return false;
        }

        if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
            LOG_CRITICAL("Failed to initialize GLAD! - '%s'\n", SDL_GetError());
            return false;
        }

        if(props.vsync) {
            if(SDL_GL_SetSwapInterval(1) < 0) {
                LOG_ERROR("SDL_GL_SetSwapInterval failed! - '%s'\n", SDL_GetError());
            }
        }

        if(props.srgb) {
            glEnable(GL_FRAMEBUFFER_SRGB);
        }

        if(props.msaaSamples > 0) {
            glEnable(GL_MULTISAMPLE);
        }

        #ifndef NDEBUG
            if(GLAD_GL_KHR_debug) {
                LOG_DEBUG("KHR_debug supported. Turning on debug output.");
                glDebugMessageCallback(debugCallback, nullptr);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
                glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
            } else if (GLAD_GL_ARB_debug_output) {
                LOG_DEBUG("ARB_debug_output supported. Turning on debug output.");
                glDebugMessageCallbackARB(debugCallback, nullptr);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
            }
        #endif

        return true;
    }

    void update() {
        SDL_Event e;
        while(SDL_PollEvent(&e) != 0) {
            switch(e.type) {
                case SDL_QUIT:
                    closeSignal.emit();
                    break;
                case SDL_WINDOWEVENT:
                    switch(e.window.event) {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            resizeSignal.emit(e.window.data1, e.window.data2);
                            break;
                    }
                    break;
            }
        }
    }

    void swapBuffers() {
        SDL_GL_SwapWindow(sdlWindow);
        #ifndef NDEBUG
            // We do this right after the buffer swap, because glGetError might
            // trigger a flush, which will have happend in when swapping buffers anyway.
            checkGLError();
        #endif
    }

    void updateAndSwap() {
        update();
        swapBuffers();
    }

    void cleanup() {
        SDL_DestroyWindow(sdlWindow);
        SDL_Quit();
    }
}
