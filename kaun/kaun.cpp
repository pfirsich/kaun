#include "kaun.hpp"

namespace kaun {
    VertexFormat defaultVertexFormat;
    RenderState defaultRenderState;

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

    void init(bool loadGl) {
        setupDefaultLogging();

        if(loadGl) {
            if(!gladLoadGL()) {
                LOG_CRITICAL("Failed to initialize GLAD!");
            }
        }

        // This should do nothing for non-srgb render targets, so I think it's fine
        // to unconditionally enable this with no control about it otherwise.
        setSrgbEnabled(true);

        // This should not do anything for non-multisamples rendertargets, so the same applies here
        // It might be useful though to render geometry without msaa to msaa render targets, but
        // I have heard that feature is broken on some hardware, so I will not bother, *for now*.
        glEnable(GL_MULTISAMPLE);

        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

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

        RenderTarget::currentRead = RenderTarget::Window::instance();
        RenderTarget::currentDraw = RenderTarget::Window::instance();

        defaultVertexFormat
            .add(kaun::AttributeType::POSITION, 3, kaun::AttributeDataType::F32)
            .add(kaun::AttributeType::NORMAL, 3, kaun::AttributeDataType::F32)
            .add(kaun::AttributeType::TEXCOORD0, 2, kaun::AttributeDataType::F32);
    }
}
