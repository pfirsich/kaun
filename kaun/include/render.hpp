#pragma once

#include <string>

#include <glm/glm.hpp>

#include "transform.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "renderstate.hpp"
#include "renderattachment.hpp"
#include "uniform.hpp"

namespace kaun {
    extern void setRenderTarget(const std::vector<const RenderAttachment*>& colorAttachments = {},
                                const RenderAttachment* depthStencil = nullptr,
                                bool blitCurrent = false);
    extern void clear(const glm::vec4& color = glm::vec4(0.0f), int colorAttachmentIndex = 0);
    extern void clearDepth(float value = 1.0f);
    extern void setViewport();
    extern void setViewport(int x, int y, int w, int h);
    extern void setViewport(const glm::ivec4& viewport);

    extern void setProjection(const glm::mat4& matrix);
    extern void setViewMatrix(const glm::mat4& viewTransform);
    extern void setViewTransform(const Transform& viewTransform);
    extern void setModelMatrix(const glm::mat4& modelTransform);
    extern void setModelTransform(const Transform& modelTransform);

    // This is a map of uniforms, because I want it to be similar to the Lua API
    // http://supercomputingblog.com/windows/ordered-map-vs-unordered-map-a-performance-study/
    extern void draw(Mesh& mesh, Shader& shader, const std::vector<Uniform>& uniforms,
                     const RenderState& state = defaultRenderState);

    enum class SortType {
        DEFAULT, // sort by shader, textures, etc.
        SUBMISSION, // sort by order of submission
    };

    extern void flush(SortType sortType = SortType::DEFAULT);

    extern void ensureGlState();
    extern void checkGLError();
}
