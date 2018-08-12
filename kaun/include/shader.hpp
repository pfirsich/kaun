#pragma once

#include <string>
#include <unordered_map>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "log.hpp"
#include "texture.hpp"
#include "uniforminfo.hpp"

namespace kaun {
    class Shader {
    public:
        enum class Type {
            VERTEX = 1,
            FRAGMENT = 2,
            GEOMETRY = 3,
        };

        enum class Status {
            EMPTY, UNLINKED, LINKED, COMPILE_SHADER_FAILED, LINK_FAILED
        };

        using UniformLocation = GLint;

    private:
        GLuint mProgramObject;
        std::vector<GLuint> mShaderObjects;
        Status mStatus;
        mutable std::unordered_map<std::string, UniformLocation> mAttributeLocations;
        mutable std::unordered_map<std::string, UniformLocation> mUniformLocations;
        std::unordered_map<std::string, UniformInfo> mUniformInfo;

        void retrieveUniformInfo();

        static const Shader* currentShaderProgram;
        static UniformInfo invalidUniform;

        static std::string globalShaderPreamble;
        static std::string fragmentShaderPreamble;
        static std::string vertexShaderPreamble;
        static std::string geometryShaderPreamble;

    public:
        static void ensureGlState();

        Shader() : mProgramObject(0), mStatus(Status::EMPTY) {}

        Shader(const std::string& fragPath, const std::string& vertPath) :
                mProgramObject(0), mStatus(Status::EMPTY) {
            compileAndLinkFiles(fragPath, vertPath);
        }

        ~Shader() {
            if(glIsProgram(mProgramObject)) {
                glDeleteProgram(mProgramObject);
            }
        }

        bool compileString(const std::string& source, Type type);

        bool compileAndLinkStrings(const std::string& frag, const std::string& vert) {
            return  compileString(frag, Type::FRAGMENT)
                    && compileString(vert, Type::VERTEX)
                    && link();
        }

        bool compileFile(const std::string& filename, Type type);

        bool compileAndLinkFiles(const std::string& fragPath, const std::string& vertPath) {
            return  compileFile(fragPath, Type::FRAGMENT)
                    && compileFile(vertPath, Type::VERTEX)
                    && link();
        }

        bool link();

        UniformLocation getAttributeLocation(const std::string& name) const;
        UniformLocation getUniformLocation(const std::string& name) const;

        const UniformInfo& getUniformInfo(const std::string& name) const;

        inline void bind() const {
            if(currentShaderProgram != this) {
                if(mProgramObject > 0) {
                    glUseProgram(mProgramObject);
                    currentShaderProgram = this;
                } else {
                    LOG_ERROR("Trying to bind shader without valid program object.");
                }
            }
        }

        // This could be static, but I want it to look like other bindables
        void unbind() const {
            glUseProgram(0);
            currentShaderProgram = nullptr;
        }

        GLuint getProgramObject() const { return mProgramObject; }

        Status getStatus() const { return mStatus; }

        template<typename... Args>
        void setUniform(const std::string& name, Args&&... args) const {
            UniformLocation loc = getUniformLocation(name);
            if(loc != -1) setUniform(loc, std::forward<Args>(args)...);
        }

        void setUniform(UniformLocation loc, int value) const {
            bind(); glUniform1i(loc, value);
        }

        void setUniform(UniformLocation loc, const int* vals, size_t count = 1) {
            bind(); glUniform1iv(loc, count, vals);
        }

        void setUniform(UniformLocation loc, float value) const {
            bind(); glUniform1f(loc, value);
        }

        void setUniform(UniformLocation loc, const float* vals, size_t count = 1) const {
            bind(); glUniform1fv(loc, count, vals);
        }

        void setUniform(UniformLocation loc, const glm::vec2& val) const {
            bind(); glUniform2fv(loc, 1, glm::value_ptr(val));
        }

        void setUniform(UniformLocation loc, const glm::vec2* vals, size_t count = 1) const {
            bind(); glUniform2fv(loc, count, glm::value_ptr(*vals));
        }

        void setUniform(UniformLocation loc, const glm::vec3& val) const {
            bind(); glUniform3fv(loc, 1, glm::value_ptr(val));
        }

        void setUniform(UniformLocation loc, const glm::vec3* vals, size_t count = 1) const {
            bind(); glUniform3fv(loc, count, glm::value_ptr(*vals));
        }

        void setUniform(UniformLocation loc, const glm::vec4& val) const {
            bind(); glUniform4fv(loc, 1, glm::value_ptr(val));
        }

        void setUniform(UniformLocation loc, const glm::vec4* vals, size_t count = 1) const {
            bind(); glUniform4fv(loc, count, glm::value_ptr(*vals));
        }

        void setUniform(UniformLocation loc, const glm::mat2& val) const {
            bind(); glUniformMatrix2fv(loc, 1, GL_FALSE, glm::value_ptr(val));
        }

        void setUniform(UniformLocation loc, const glm::mat2* vals, size_t count = 1) const {
            bind(); glUniformMatrix2fv(loc, count, GL_FALSE, glm::value_ptr(*vals));
        }

        void setUniform(UniformLocation loc, const glm::mat3& val) const {
            bind(); glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(val));
        }

        void setUniform(UniformLocation loc, const glm::mat3* vals, size_t count = 1) const {
            bind(); glUniformMatrix3fv(loc, count, GL_FALSE, glm::value_ptr(*vals));
        }

        void setUniform(UniformLocation loc, const glm::mat4& val) const {
            bind(); glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val));
        }

        void setUniform(UniformLocation loc, const glm::mat4* vals, size_t count = 1) const {
            bind(); glUniformMatrix4fv(loc, count, GL_FALSE, glm::value_ptr(*vals));
        }

        void setUniform(UniformLocation loc, const Texture& tex) const {
            bind();
            int unit = tex.bind();
            glUniform1i(loc, unit);
        }

        void setUniform(UniformLocation loc, const Texture& tex, int unit) const {
            bind();
            tex.bind(unit);
            glUniform1i(loc, unit);
        }
    };
}
