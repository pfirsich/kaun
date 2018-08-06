#pragma once

#include <string>

#include <glm/glm.hpp>

#include "transform.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "renderstate.hpp"
#include "renderattachment.hpp"

namespace kaun {
    extern void setRenderTarget(const std::vector<const RenderAttachment*>& colorAttachments = {},
                                const RenderAttachment* depthStencil = nullptr);
    extern void clear(const glm::vec4& color = glm::vec4(0.0f), int colorAttachmentIndex = 0);
    extern void clearDepth(float value = 1.0f);
    extern void setViewport(int x, int y, int w, int h);
    extern void setViewport(const glm::ivec4& viewport);

    extern void setProjection(const glm::mat4& matrix);
    extern void setViewTransform(const Transform& viewTransform);
    extern void setModelTransform(const Transform& modelTransform);

    //extern void draw(Mesh& mesh, Shader& shader, const std::map<Uniform>& uniforms = {},
    //                 const RenderState& state = defaultRenderState);

    extern void flush(/*sortFunction - pointer/enum?*/);
}
