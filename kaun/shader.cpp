#include <fstream>
#include <memory>
#include <sstream>
#include <vector>
#include <unordered_map>

#include <glad/glad.h>

#include "shader.hpp"
#include "log.hpp"
#include "utility.hpp"

namespace kaun {
    const Shader* Shader::currentShaderProgram = nullptr;
    UniformInfo Shader::invalidUniform;

    void Shader::ensureGlState() {
        if(currentShaderProgram == nullptr) {
            glUseProgram(0);
        } else {
            glUseProgram(currentShaderProgram->getProgramObject());
        }
    }

    bool Shader::compileString(const std::string& source, Shader::Type type) {
        //LOG_DEBUG("%s:\n%s", type == ShaderType::FRAGMENT ? "fragment" : "vertex", source);

        if(mStatus != Status::EMPTY && mStatus != Status::UNLINKED) {
            LOG_ERROR("To compile and attach a shader, the status must be EMPTY or UNLINKED");
        }

        std::string fullSource;

        GLenum GLtype = 0;
        if(type == Shader::Type::VERTEX) {
            GLtype = GL_VERTEX_SHADER;
            fullSource = globalShaderPreamble + vertexShaderPreamble + source;
        } else if(type == Shader::Type::FRAGMENT) {
            GLtype = GL_FRAGMENT_SHADER;
            fullSource = globalShaderPreamble + fragmentShaderPreamble + source;
        } else if(type == Shader::Type::GEOMETRY) {
            GLtype = GL_GEOMETRY_SHADER;
            fullSource = globalShaderPreamble + geometryShaderPreamble + source;
        } else {
            LOG_ERROR("Unknown shader type");
            return false;
        }

        GLuint shader = glCreateShader(GLtype);
        //LOG_DEBUG(source);
		const char* cStr = fullSource.c_str();
        glShaderSource(shader, 1, &cStr, nullptr);

        GLint compileStatus;
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
        if(compileStatus == GL_FALSE){
            mStatus = Status::COMPILE_SHADER_FAILED;
            GLint logLength;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
            std::unique_ptr<GLchar> shaderLog(new GLchar[logLength]);
            glGetShaderInfoLog(shader, logLength, nullptr, shaderLog.get());
            LOG_ERROR("shader compile failed: %s", shaderLog.get());
            return false;
        } else {
            mStatus = Status::UNLINKED;
            mShaderObjects.push_back(shader);
            return true;
        }
    }

    bool Shader::compileFile(const std::string& filename, Shader::Type type) {
        auto fileData = readFile(filename);
        if(fileData) {
            return compileString(fileData->c_str(), type);
        } else {
            LOG_ERROR("Shader file '%s' could not be opened.", filename.c_str());
            return false;
        }
    }

    void Shader::retrieveUniformInfo() {
        GLint maxUniformNameLength;
        glGetProgramiv(mProgramObject,  GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);
        char* name = new char[maxUniformNameLength];
        GLsizei length;
        GLint size;
        GLenum type;
        GLint activeUniformCount;
        glGetProgramiv(mProgramObject, GL_ACTIVE_UNIFORMS, &activeUniformCount);
        for(int i = 0; i < activeUniformCount; ++i) {
            glGetActiveUniform(mProgramObject, i, maxUniformNameLength, &length, &size, &type, name);
            if(length > 0) {
                mUniformInfo[name] = UniformInfo(i, size, static_cast<UniformInfo::UniformType>(type), name);
            }
        }
        delete[] name;
    }

    const UniformInfo& Shader::getUniformInfo(const std::string& name) const {
        auto it = mUniformInfo.find(name);
        if(it == mUniformInfo.end()) {
            return invalidUniform;
        } else {
            return it->second;
        }
    }

    bool Shader::link() {
        if(mStatus != Status::UNLINKED) {
            LOG_ERROR("To link a shader program, the status must be UNLINKED");
            return false;
        }

        if(mShaderObjects.size() == 0) {
            LOG_ERROR("Shader program can not be linked without shader objects");
            return false;
        }

        mProgramObject = glCreateProgram();
        for(auto shader: mShaderObjects) {
            glAttachShader(mProgramObject, shader);
        }
        glLinkProgram(mProgramObject);

        for(auto shader: mShaderObjects) {
            glDetachShader(mProgramObject, shader);
            glDeleteShader(shader);
        }
        mShaderObjects.clear();

        GLint linkStatus;
        glGetProgramiv(mProgramObject, GL_LINK_STATUS, &linkStatus);
        if(linkStatus == GL_FALSE) {
            mStatus = Status::LINK_FAILED;
            GLint logLength = 0;
            glGetProgramiv(mProgramObject, GL_INFO_LOG_LENGTH, &logLength);
            std::unique_ptr<GLchar> programLog(new GLchar[logLength]);
            glGetProgramInfoLog(mProgramObject, logLength, nullptr, programLog.get());
            LOG_ERROR("Program link failed: %s", programLog.get());
            return false;
        } else {
            mStatus = Status::LINKED;
            retrieveUniformInfo();
            return true;
        }
    }

    GLint Shader::getAttributeLocation(const std::string& name) const {
        auto it = mAttributeLocations.find(name);
        if(it == mAttributeLocations.end()) {
            GLint loc = glGetAttribLocation(mProgramObject, name.c_str());
            if(loc == -1) {
                LOG_WARNING("Attribute '%s' does not exist in shader program.", name.c_str());
            }
            mAttributeLocations[name] = loc;
            return loc;
        } else {
            return it->second;
        }
    }

    GLint Shader::getUniformLocation(const std::string& name) const {
        auto it = mUniformLocations.find(name);
        if(it == mUniformLocations.end()) {
            GLint loc = glGetUniformLocation(mProgramObject, name.c_str());
            if(loc == -1) {
                LOG_WARNING("Uniform '%s' does not exist in shader program.", name.c_str());
            }
            mUniformLocations[name] = loc;
            return loc;
        } else {
            return it->second;
        }
    }
}
