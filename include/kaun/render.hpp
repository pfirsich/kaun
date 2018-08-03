#pragma once

#include <string>

#include <glm/glm.hpp>

#include "transform.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "uniform.hpp"
#include "rendertarget.hpp"
#include "renderstate.hpp"

namespace kaun {
    extern void setProjection(const glm::mat4& matrix);
    extern void setViewTransform(const Transform& viewTransform);
    extern void setModelTransform(const Transform& modelTransform);

    //extern void setRenderTarget(RenderTarget& renderTarget);
    extern void getRenderState(RenderState& state);
    extern void setRenderState(RenderState& state);
    extern void setShader(Shader& shader);

    extern void setUniform(const std::string& name, float val);
    extern void setUniform(const std::string& name, const float* vals, size_t count = 1);

    extern void setUniform(const std::string& name, int val);
    extern void setUniform(const std::string& name, const int* vals, size_t count = 1);

    extern void setUniform(const std::string& name, const glm::vec2& val);
    extern void setUniform(const std::string& name, const glm::vec2* vals, size_t count = 1);

    extern void setUniform(const std::string& name, const glm::vec3& val);
    extern void setUniform(const std::string& name, const glm::vec3* vals, size_t count = 1);

    extern void setUniform(const std::string& name, const glm::vec4& val);
    extern void setUniform(const std::string& name, const glm::vec4* vals, size_t count = 1);

    extern void setUniform(const std::string& name, const glm::mat2& val);
    extern void setUniform(const std::string& name, const glm::mat2* vals, size_t count = 1);

    extern void setUniform(const std::string& name, const glm::mat3& val);
    extern void setUniform(const std::string& name, const glm::mat3* vals, size_t count = 1);

    extern void setUniform(const std::string& name, const glm::mat4& val);
    extern void setUniform(const std::string& name, const glm::mat4* vals, size_t count = 1);

    extern void draw(Mesh& mesh);

    extern void flush();
}
