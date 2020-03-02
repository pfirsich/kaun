#include <algorithm>
#include <vector>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "render.hpp"
#include "rendertarget.hpp"

namespace kaun {
glm::ivec4 viewport;
bool currentSrgbEnabled = false;

void clear(const glm::vec4& color, int colorAttachmentIndex)
{
    glClearBufferfv(GL_COLOR, colorAttachmentIndex, glm::value_ptr(color));
}

void clearDepth(float value)
{
    glClearBufferfv(GL_DEPTH, 0, &value);
}

void setViewport()
{
    setViewport(viewport.x, viewport.y, viewport.z, viewport.w);
}

void setViewport(int x, int y, int w, int h)
{
    viewport = glm::ivec4(x, y, w, h);
    glViewport(x, y, w, h);
}

void setViewport(const glm::ivec4& vp)
{
    setViewport(vp.x, vp.y, vp.z, vp.w);
}

void setSrgbEnabled(bool enabled)
{
    if (enabled) {
        glEnable(GL_FRAMEBUFFER_SRGB);
    } else {
        glDisable(GL_FRAMEBUFFER_SRGB);
    }
    currentSrgbEnabled = enabled;
}

bool getSrgbEnabled()
{
    return currentSrgbEnabled;
}

glm::mat4 projectionMatrix;
glm::mat4 invProjectionMatrix;
glm::mat4 viewMatrix;
glm::mat4 invViewMatrix;
glm::mat4 viewProjectionMatrix;
glm::mat4 invViewProjectionMatrix;
glm::mat4 modelMatrix;
glm::mat3 normalMatrix;

struct RenderQueueEntry {
    Mesh* mesh;
    Shader* shader;
    std::vector<Uniform> uniforms;
    RenderState renderState;
    float depth;
    uint64_t sortKey;

    RenderQueueEntry(Mesh* mesh, Shader* shader, const RenderState& renderState)
        : mesh(mesh)
        , shader(shader)
        , renderState(renderState)
        , sortKey(0)
    {
    }
};

std::vector<RenderQueueEntry> renderQueue;

void updateViewProjection()
{
    viewProjectionMatrix = projectionMatrix * viewMatrix;
    invViewProjectionMatrix = invViewMatrix * invProjectionMatrix;
}

void setProjection(const glm::mat4& matrix)
{
    projectionMatrix = matrix;
    invProjectionMatrix = glm::inverse(projectionMatrix);
    updateViewProjection();
}

glm::mat4 getProjection()
{
    return projectionMatrix;
}

void setViewMatrix(const glm::mat4& view)
{
    viewMatrix = view;
    invViewMatrix = glm::inverse(viewMatrix);
    updateViewProjection();
}

void setViewTransform(const Transform& viewTransform)
{
    invViewMatrix = viewTransform.getMatrix();
    viewMatrix = glm::inverse(invViewMatrix);
    updateViewProjection();
}

glm::mat4 getViewMatrix()
{
    return viewMatrix;
}

void setModelMatrix(const glm::mat4& model)
{
    modelMatrix = model;
    normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
}

void setModelTransform(const Transform& modelTransform)
{
    setModelMatrix(modelTransform.getMatrix());
}

glm::mat4 getModelMatrix()
{
    return modelMatrix;
}

void draw(
    Mesh& mesh, Shader& shader, const std::vector<Uniform>& uniforms, const RenderState& state)
{
    renderQueue.emplace_back(&mesh, &shader, state);
    RenderQueueEntry& entry = renderQueue.back();

    glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
    glm::mat4 modelViewProjectionMatrix = viewProjectionMatrix * modelMatrix;
    glm::vec4 projected = modelViewProjectionMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    entry.depth = projected.z / projected.w;

    // insert built-in uniforms first, so we know where to find them in the vector
    entry.uniforms.emplace_back("kaun_viewport", viewport);
    entry.uniforms.emplace_back("kaun_view", viewMatrix);
    entry.uniforms.emplace_back("kaun_invView", invViewMatrix);
    entry.uniforms.emplace_back("kaun_projection", projectionMatrix);
    entry.uniforms.emplace_back("kaun_invProjection", invProjectionMatrix);
    entry.uniforms.emplace_back("kaun_viewProjection", viewProjectionMatrix);
    entry.uniforms.emplace_back("kaun_invViewProjection", invViewProjectionMatrix);
    entry.uniforms.emplace_back("kaun_model", modelMatrix);
    entry.uniforms.emplace_back("kaun_normal", normalMatrix);
    entry.uniforms.emplace_back("kaun_modelView", modelViewMatrix);
    entry.uniforms.emplace_back("kaun_modelViewProjection", modelViewProjectionMatrix);

    entry.uniforms.reserve(entry.uniforms.size() + uniforms.size());
    entry.uniforms.insert(entry.uniforms.end(), uniforms.begin(), uniforms.end());
}

bool entryCompare(const RenderQueueEntry& a, const RenderQueueEntry& b)
{
    return a.sortKey < b.sortKey;
}

uint64_t defaultSortKey(const RenderQueueEntry& entry)
{
    // draw opaque geometry first => translucencyType = 0
    uint64_t translucencyType = entry.renderState.getBlendEnabled() ? 1 : 0;
    uint64_t shader = entry.shader->getProgramObject();
    uint64_t depth = static_cast<uint64_t>(entry.depth * 0xFFFFFF);
    if (translucencyType > 0)
        depth = 0xFFFFFF - depth;
    return (translucencyType << 63) | (shader & 0xFFFFFF) << 24 | depth;
}

void flush(SortType sortType)
{
    static std::vector<const Texture*> textures;

    switch (sortType) {
    case SortType::DEFAULT:
        for (auto& entry : renderQueue)
            entry.sortKey = defaultSortKey(entry);
        std::sort(renderQueue.begin(), renderQueue.end(), entryCompare);
        break;
    case SortType::SUBMISSION:
        // don't do anything
        break;
    }

    for (auto& entry : renderQueue) {
        entry.renderState.apply();

        textures.clear();
        for (auto& uniform : entry.uniforms) {
            if (uniform.getType() == Uniform::Type::TEXTURE)
                textures.push_back(uniform.getTexture());
        }
        Texture::bindTextures(textures);

        entry.shader->bind();
        for (auto& uniform : entry.uniforms) {
            Shader::UniformLocation loc
                = entry.shader->getUniformLocation(uniform.getName(), false);
            if (loc != -1)
                uniform.set(loc);
        }
        entry.mesh->draw();
    }
    renderQueue.clear();

#ifndef NDEBUG
    checkGlError();
#endif
}

void ensureGlState()
{
    setSrgbEnabled(getSrgbEnabled());
    RenderTarget::ensureGlState();
    RenderState::ensureGlState();
    Shader::ensureGlState();
    Texture::ensureGlState();
    Mesh::ensureGlState();
}

void checkGlError()
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::string text("Unknown error");
        switch (err) {
        case GL_INVALID_ENUM:
            text = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            text = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            text = "GL_INVALID_OPERATION";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            text = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            text = "GL_OUT_OF_MEMORY";
            break;
        }
        LOG_WARNING("GL Error!: 0x%X - %s\n", err, text.c_str());
    }
}
}
