#pragma once

#include <string>

#include <glad/glad.h>

namespace kaun {
class UniformInfo {
public:
    enum class UniformType : GLenum {
        NONE = 0,
        FLOAT = GL_FLOAT,
        VEC2 = GL_FLOAT_VEC2,
        VEC3 = GL_FLOAT_VEC3,
        VEC4 = GL_FLOAT_VEC4,
        DOUBLE = GL_DOUBLE,
        INT = GL_INT,
        IVEC2 = GL_INT_VEC2,
        IVEC3 = GL_INT_VEC3,
        IVEC4 = GL_INT_VEC4,
        UNSIGNED_INT = GL_UNSIGNED_INT,
        UVEC2 = GL_UNSIGNED_INT_VEC2,
        UVEC3 = GL_UNSIGNED_INT_VEC3,
        UVEC4 = GL_UNSIGNED_INT_VEC4,
        BOOL = GL_BOOL,
        BVEC2 = GL_BOOL_VEC2,
        BVEC3 = GL_BOOL_VEC3,
        BVEC4 = GL_BOOL_VEC4,
        MAT2 = GL_FLOAT_MAT2,
        MAT3 = GL_FLOAT_MAT3,
        MAT4 = GL_FLOAT_MAT4,
        MAT2X3 = GL_FLOAT_MAT2x3,
        MAT2X4 = GL_FLOAT_MAT2x4,
        MAT3X2 = GL_FLOAT_MAT3x2,
        MAT3X4 = GL_FLOAT_MAT3x4,
        MAT4X2 = GL_FLOAT_MAT4x2,
        MAT4X3 = GL_FLOAT_MAT4x3,
        SAMPLER1D = GL_SAMPLER_1D,
        SAMPLER2D = GL_SAMPLER_2D,
        SAMPLER3D = GL_SAMPLER_3D,
        SAMPLERCUBE = GL_SAMPLER_CUBE,
        SAMPLER1DSHADOW = GL_SAMPLER_1D_SHADOW,
        SAMPLER2DSHADOW = GL_SAMPLER_2D_SHADOW,
        SAMPLER1DARRAY = GL_SAMPLER_1D_ARRAY,
        SAMPLER2DARRAY = GL_SAMPLER_2D_ARRAY,
        SAMPLER1DARRAYSHADOW = GL_SAMPLER_1D_ARRAY_SHADOW,
        SAMPLER2DARRAYSHADOW = GL_SAMPLER_2D_ARRAY_SHADOW,
        SAMPLER2DMS = GL_SAMPLER_2D_MULTISAMPLE,
        SAMPLER2DMSARRAY = GL_SAMPLER_2D_MULTISAMPLE_ARRAY,
        SAMPLERCUBESHADOW = GL_SAMPLER_CUBE_SHADOW,
        SAMPLERBUFFER = GL_SAMPLER_BUFFER,
        SAMPLER2DRECT = GL_SAMPLER_2D_RECT,
        SAMPLER2DRECTSHADOW = GL_SAMPLER_2D_RECT_SHADOW,
        ISAMPLER1D = GL_INT_SAMPLER_1D,
        ISAMPLER2D = GL_INT_SAMPLER_2D,
        ISAMPLER3D = GL_INT_SAMPLER_3D,
        ISAMPLERCUBE = GL_INT_SAMPLER_CUBE,
        ISAMPLER1DARRAY = GL_INT_SAMPLER_1D_ARRAY,
        ISAMPLER2DARRAY = GL_INT_SAMPLER_2D_ARRAY,
        ISAMPLER2DMS = GL_INT_SAMPLER_2D_MULTISAMPLE,
        ISAMPLER2DMSARRAY = GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
        ISAMPLERBUFFER = GL_INT_SAMPLER_BUFFER,
        ISAMPLER2DRECT = GL_INT_SAMPLER_2D_RECT,
        USAMPLER1D = GL_UNSIGNED_INT_SAMPLER_1D,
        USAMPLER2D = GL_UNSIGNED_INT_SAMPLER_2D,
        USAMPLER3D = GL_UNSIGNED_INT_SAMPLER_3D,
        USAMPLERCUBE = GL_UNSIGNED_INT_SAMPLER_CUBE,
        USAMPLER1DARRAY = GL_UNSIGNED_INT_SAMPLER_1D_ARRAY,
        USAMPLER2DARRAY = GL_UNSIGNED_INT_SAMPLER_2D_ARRAY,
        USAMPLER2DMS = GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE,
        USAMPLER2DMSARRAY = GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,
        USAMPLERBUFFER = GL_UNSIGNED_INT_SAMPLER_BUFFER,
        USAMPLER2DRECT = GL_UNSIGNED_INT_SAMPLER_2D_RECT
    };

private:
    int mIndex;
    int mSize;
    UniformType mType;
    std::string mName;

public:
    UniformInfo()
        : mIndex(-1)
        , mSize(0)
        , mType(UniformType::NONE)
        , mName("")
    {
    }
    UniformInfo(int index, int size, UniformType type, const std::string& name)
        : mIndex(index)
        , mSize(size)
        , mType(type)
        , mName(name)
    {
    }

    bool exists() const
    {
        return mType != UniformType::NONE;
    }
    int getIndex() const
    {
        return mIndex;
    }
    int getSize() const
    {
        return mSize;
    }
    UniformType getType() const
    {
        return mType;
    }
    std::string getName() const
    {
        return mName;
    }
};
}
