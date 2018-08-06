#pragma once

#include <string>
#include <vector>

#include <glad/glad.h>

namespace kaun {
    enum class AttributeType : int {
        POSITION = 0,
        NORMAL,
        TANGENT,
        BITANGENT,
        COLOR0,
        COLOR1,
        BONEINDICES,
        BONEWEIGHTS,
        TEXCOORD0, TEXCOORD1, TEXCOORD2, TEXCOORD3, // 8
        CUSTOM0, CUSTOM1, CUSTOM2, CUSTOM3, CUSTOM4, CUSTOM5, CUSTOM6, CUSTOM7, // 12
        FINAL_COUNT_ENTRY // This is not a real type, just used to count
    };

    const char* getVertexAttributeTypeName(AttributeType attrType);

    enum class AttributeDataType : GLenum {
        I8 = GL_BYTE,
        UI8 = GL_UNSIGNED_BYTE,
        I16 = GL_SHORT,
        UI16 = GL_UNSIGNED_SHORT,
        I32 = GL_INT,
        UI32 = GL_UNSIGNED_INT,
        //F16 = GL_HALF_FLOAT,
        F32 = GL_FLOAT,
        //F64 = GL_DOUBLE,
        I2_10_10_10 = GL_INT_2_10_10_10_REV,
        UI2_10_10_10 = GL_UNSIGNED_INT_2_10_10_10_REV,
    };

    int getAttributeDataTypeSize(AttributeDataType type);

	struct VertexAttribute {
		AttributeType type;
		int num, alignedNum;
		AttributeDataType dataType;
		int offset;
		bool normalized;
		unsigned int divisor;

		VertexAttribute(AttributeType attrType, int num, AttributeDataType dataType,
			int offset, bool normalized = false, unsigned int divisor = 0);
	};

    struct VertexFormat {
    private:
        std::vector<VertexAttribute> mAttributes;
        int mStride;

    public:
        VertexFormat() : mStride(0) {}

        const std::vector<VertexAttribute>& getAttributes() const {return mAttributes;}

        int getAttributeCount() const {return mAttributes.size();}

        const VertexAttribute* getAttribute(AttributeType attrType) const;

        // maybe add hasAttributes? probably pass a vector to that, but
        // that is more hassle than gain
        bool hasAttribute(AttributeType attrType) const;

        int getStride() const { return mStride; }

        // returns *this, so you can chain them
        VertexFormat& add(AttributeType attrType, int num, AttributeDataType dataType, bool normalized = false, unsigned int divisor = 0);
    };

    extern VertexFormat defaultVertexFormat;
}
