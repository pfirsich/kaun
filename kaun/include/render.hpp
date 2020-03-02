#pragma once

#include <string>

#include <glm/glm.hpp>

#include "mesh.hpp"
#include "renderstate.hpp"
#include "shader.hpp"
#include "transform.hpp"
#include "uniform.hpp"

namespace kaun {
void clear(const glm::vec4& color = glm::vec4(0.0f), int colorAttachmentIndex = 0);
void clearDepth(float value = 1.0f);
void setViewport();
void setViewport(int x, int y, int w, int h);
void setViewport(const glm::ivec4& viewport);
void setSrgbEnabled(bool enabled);
bool getSrgbEnabled();

void setProjection(const glm::mat4& matrix);
glm::mat4 getProjection();
void setViewMatrix(const glm::mat4& viewTransform);
void setViewTransform(const Transform& viewTransform);
glm::mat4 getViewMatrix();
void setModelMatrix(const glm::mat4& modelTransform);
void setModelTransform(const Transform& modelTransform);
glm::mat4 getModelMatrix();

// This is a map of uniforms, because I want it to be similar to the Lua API
// http://supercomputingblog.com/windows/ordered-map-vs-unordered-map-a-performance-study/
void draw(Mesh& mesh, Shader& shader, const std::vector<Uniform>& uniforms,
    const RenderState& state = defaultRenderState);

enum class SortType {
    DEFAULT, // sort by shader, textures, etc.
    SUBMISSION, // sort by order of submission
};

void flush(SortType sortType = SortType::DEFAULT);

void ensureGlState();
void checkGlError();
}
