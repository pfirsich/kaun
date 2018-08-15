#include <memory>
#include <variant>

#include "shader.hpp"
#include "texture.hpp"

namespace kaun {
    class Uniform {
    public:
        enum class Type {
            FLOAT, VEC2F, VEC3F, VEC4F,
            INT, VEC2I, VEC3I, VEC4I,
            UINT, VEC2UI, VEC3UI, VEC4UI,
            MAT2, MAT3, MAT4,
            MAT2x3, MAT3x2, MAT2x4, MAT4x2, MAT3x4, MAT4x3,
            TEXTURE
        };

    private:
        std::string mName;
        Type mType;
        int mCount;
        using DataPtr = std::shared_ptr<const uint8_t>;
        std::variant<const Texture*, DataPtr> mData;

        int getTypeSize() { // in bytes
            switch(mType) {
                case Type::FLOAT: return 4;
                case Type::VEC2F: return 4*2;
                case Type::VEC3F: return 4*3;
                case Type::VEC4F: return 4*4;
                case Type::INT: return 4;
                case Type::VEC2I: return 4*2;
                case Type::VEC3I: return 4*3;
                case Type::VEC4I: return 4*4;
                case Type::UINT: return 4;
                case Type::VEC2UI: return 4*2;
                case Type::VEC3UI: return 4*3;
                case Type::VEC4UI: return 4*4;
                case Type::MAT2: return 4*2*2;
                case Type::MAT3: return 4*3*3;
                case Type::MAT4: return 4*4*4;
                case Type::MAT2x3: return 4*2*3;
                case Type::MAT3x2: return 4*3*2;
                case Type::MAT2x4: return 4*2*4;
                case Type::MAT4x2: return 4*4*2;
                case Type::MAT3x4: return 4*3*4;
                case Type::MAT4x3: return 4*4*3;
                case Type::TEXTURE: return 4; // just an int
            }
            return 0;
        }

        void copyData(const void* data) {
            int size = getTypeSize() * mCount;
            uint8_t* temp = new uint8_t[size];
            std::memcpy(temp, data, size);
            mData = DataPtr(temp);
        }

        template <typename T>
        const T* getData() const {
            return reinterpret_cast<const T*>(std::get<DataPtr>(mData).get());
        }

    public:
        // Uniform will copy the data on construction
        // since the data is "safe" there, the copy constructor will just copy the pointer.
        Uniform(const Uniform& other) = default;

        Uniform(const std::string& name, int val) :
            mName(name), mType(Type::INT), mCount(1) { copyData(&val); }

        Uniform(const std::string& name, const int* vals, size_t count = 1) :
            mName(name), mType(Type::INT), mCount(count) { copyData(vals); }

        Uniform(const std::string& name, float val) :
            mName(name), mType(Type::FLOAT), mCount(1) { copyData(&val); }

        Uniform(const std::string& name, const float* vals, size_t count = 1) :
            mName(name), mType(Type::FLOAT), mCount(count) { copyData(vals); }

        Uniform(const std::string& name, const glm::vec2& val) :
            mName(name), mType(Type::VEC2F), mCount(1) { copyData(glm::value_ptr(val)); }

        Uniform(const std::string& name, const glm::vec2* vals, size_t count = 1) :
            mName(name), mType(Type::VEC2F), mCount(count) { copyData(glm::value_ptr(vals[0])); }

        Uniform(const std::string& name, const glm::vec3& val) :
            mName(name), mType(Type::VEC3F), mCount(1) { copyData(glm::value_ptr(val)); }

        Uniform(const std::string& name, const glm::vec3* vals, size_t count = 1) :
            mName(name), mType(Type::VEC3F), mCount(count) { copyData(glm::value_ptr(vals[0])); }

        Uniform(const std::string& name, const glm::vec4& val) :
            mName(name), mType(Type::VEC4F), mCount(1) { copyData(glm::value_ptr(val)); }

        Uniform(const std::string& name, const glm::ivec2& val) :
            mName(name), mType(Type::VEC2I), mCount(1) { copyData(glm::value_ptr(val)); }

        Uniform(const std::string& name, const glm::ivec2* vals, size_t count = 1) :
            mName(name), mType(Type::VEC2I), mCount(count) { copyData(glm::value_ptr(vals[0])); }

        Uniform(const std::string& name, const glm::ivec3& val) :
            mName(name), mType(Type::VEC3I), mCount(1) { copyData(glm::value_ptr(val)); }

        Uniform(const std::string& name, const glm::ivec3* vals, size_t count = 1) :
            mName(name), mType(Type::VEC3I), mCount(count) { copyData(glm::value_ptr(vals[0])); }

        Uniform(const std::string& name, const glm::ivec4& val) :
            mName(name), mType(Type::VEC4I), mCount(1) { copyData(glm::value_ptr(val)); }

        Uniform(const std::string& name, const glm::ivec4* vals, size_t count = 1) :
            mName(name), mType(Type::VEC4I), mCount(count) { copyData(glm::value_ptr(vals[0])); }

        Uniform(const std::string& name, const glm::mat2& val) :
            mName(name), mType(Type::MAT2), mCount(1) { copyData(glm::value_ptr(val)); }

        Uniform(const std::string& name, const glm::mat2* vals, size_t count = 1) :
            mName(name), mType(Type::MAT2), mCount(count) { copyData(glm::value_ptr(vals[0])); }

        Uniform(const std::string& name, const glm::mat3& val) :
            mName(name), mType(Type::MAT3), mCount(1) { copyData(glm::value_ptr(val)); }

        Uniform(const std::string& name, const glm::mat3* vals, size_t count = 1) :
            mName(name), mType(Type::MAT3), mCount(count) { copyData(glm::value_ptr(vals[0])); }

        Uniform(const std::string& name, const glm::mat4& val) :
            mName(name), mType(Type::MAT4), mCount(1) { copyData(glm::value_ptr(val)); }

        Uniform(const std::string& name, const glm::mat4* vals, size_t count = 1) :
            mName(name), mType(Type::MAT4), mCount(count) { copyData(glm::value_ptr(vals[0])); }

        // Deleting the texture before flush() means trouble
        Uniform(const std::string& name, const Texture& tex) :
            mName(name), mType(Type::TEXTURE), mCount(0), mData(&tex) {}

        // Do I need the following function? If yes, store unit in mCount (kind of hackish though)?
        // Uniform(const std::string& name, const Texture& tex, int unit);

        Uniform& operator=(const Uniform& other) = default;

        const std::string& getName() const { return mName; }
        Type getType() const { return mType; }
        int getCount() const { return mCount; }
        const Texture* getTexture() const {
            assert(mType == Type::TEXTURE);
            return std::get<const Texture*>(mData);
        }

        void set(Shader::UniformLocation loc) const {
            int c = mCount;
            switch(mType) {
                case Type::FLOAT:
                    glUniform1fv(loc, c, getData<float>()); break;
                case Type::VEC2F:
                    glUniform2fv(loc, c, getData<float>()); break;
                case Type::VEC3F:
                    glUniform3fv(loc, c, getData<float>()); break;
                case Type::VEC4F:
                    glUniform4fv(loc, c, getData<float>()); break;
                case Type::INT:
                    glUniform1iv(loc, c, getData<int>()); break;
                case Type::VEC2I:
                    glUniform2iv(loc, c, getData<int>()); break;
                case Type::VEC3I:
                    glUniform3iv(loc, c, getData<int>()); break;
                case Type::VEC4I:
                    glUniform4iv(loc, c, getData<int>()); break;
                case Type::UINT:
                    glUniform1uiv(loc, c, getData<unsigned int>()); break;
                case Type::VEC2UI:
                    glUniform2uiv(loc, c, getData<unsigned int>()); break;
                case Type::VEC3UI:
                    glUniform3uiv(loc, c, getData<unsigned int>()); break;
                case Type::VEC4UI:
                    glUniform4uiv(loc, c, getData<unsigned int>()); break;
                case Type::MAT2:
                    glUniformMatrix2fv(loc, c, GL_FALSE, getData<float>()); break;
                case Type::MAT3:
                    glUniformMatrix3fv(loc, c, GL_FALSE, getData<float>()); break;
                case Type::MAT4:
                    glUniformMatrix4fv(loc, c, GL_FALSE, getData<float>()); break;
                case Type::MAT2x3:
                    glUniformMatrix2x3fv(loc, c, GL_FALSE, getData<float>()); break;
                case Type::MAT3x2:
                    glUniformMatrix3x2fv(loc, c, GL_FALSE, getData<float>()); break;
                case Type::MAT2x4:
                    glUniformMatrix2x4fv(loc, c, GL_FALSE, getData<float>()); break;
                case Type::MAT4x2:
                    glUniformMatrix4x2fv(loc, c, GL_FALSE, getData<float>()); break;
                case Type::MAT3x4:
                    glUniformMatrix3x4fv(loc, c, GL_FALSE, getData<float>()); break;
                case Type::MAT4x3:
                    glUniformMatrix4x3fv(loc, c, GL_FALSE, getData<float>()); break;
                case Type::TEXTURE: {
                    int unit = getTexture()->getUnit();
                    assert(unit >= 0);
                    glUniform1i(loc, unit);
                    break;
                }
            }
        }
    };
}

