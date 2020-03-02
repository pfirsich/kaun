#include "mesh_vertexformat.hpp"

#include "log.hpp"

namespace kaun {
const char* attrTypeNames[] = {
    "POSITION",
    "NORMAL",
    "TANGENT",
    "BITANGENT",
    "COLOR0",
    "COLOR1",
    "BONEINDICES",
    "BONEWEIGHTS",
    "TEXCOORD0",
    "TEXCOORD1",
    "TEXCOORD2",
    "TEXCOORD3",
    "CUSTOM0",
    "CUSTOM1",
    "CUSTOM2",
    "CUSTOM3",
    "CUSTOM4",
    "CUSTOM5",
    "CUSTOM6",
    "CUSTOM7",
};

static_assert(static_cast<int>(AttributeType::FINAL_COUNT_ENTRY)
        == sizeof(attrTypeNames) / sizeof(attrTypeNames[0]),
    "There should be just as many AttributeType names as there are AttributeTypes");

const char* getVertexAttributeTypeName(AttributeType attrType)
{
    return attrTypeNames[static_cast<int>(attrType)];
}

int getAttributeDataTypeSize(AttributeDataType type)
{
    switch (type) {
    case AttributeDataType::I8:
    case AttributeDataType::UI8:
        return 1;
        break;
    case AttributeDataType::I16:
    case AttributeDataType::UI16:
        return 2;
        break;
    default:
        return 4;
        break;
    }
}

VertexAttribute::VertexAttribute(AttributeType attrType, int num, AttributeDataType dataType,
    int offset, bool normalized, unsigned int divisor)
    : type(attrType)
    , num(num)
    , alignedNum(num)
    , dataType(dataType)
    , offset(offset)
    , normalized(normalized)
    , divisor(divisor)
{
    // Align to 4 Bytes
    // https://www.opengl.org/wiki/Vertex_Specification_Best_Practices#Attribute_sizes
    int overlap = 0;
    switch (dataType) {
    case AttributeDataType::I8:
    case AttributeDataType::UI8:
        overlap = num % 4;
        if (overlap > 0)
            alignedNum = num + (4 - overlap);
        break;
    case AttributeDataType::I16:
    case AttributeDataType::UI16:
        overlap = (num * 2) % 4;
        if (overlap > 0)
            alignedNum = num + (4 - overlap) / 2;
        break;
    default:
        // all others should already be multiples of 4
        break;
    }
}

const VertexAttribute* VertexFormat::getAttribute(AttributeType attrType) const
{
    for (auto& attr : mAttributes) {
        if (attr.type == attrType) {
            return &attr;
        }
    }
    return nullptr;
}

VertexFormat& VertexFormat::add(AttributeType attrType, int num, AttributeDataType dataType,
    bool normalized, unsigned int divisor)
{
    if (hasAttribute(attrType)) {
        LOG_ERROR("You are trying to add an attribute to a vertex format that is already present.");
    } else {
        mAttributes.emplace_back(attrType, num, dataType, normalized, divisor);
        VertexAttribute& attr = mAttributes.back();
        attr.offset = mStride;
        mStride += getAttributeDataTypeSize(attr.dataType) * attr.alignedNum;
    }
    return *this;
}

bool VertexFormat::hasAttribute(AttributeType attrType) const
{
    for (auto& attr : mAttributes)
        if (attr.type == attrType)
            return true;
    return false;
}
}
