#include "mesh_vertexaccessor.hpp"

namespace kaun {
template <>
float VertexAttributeAccessor<float>::getInternal(size_t index) const
{
    assert(mAttr->num == 1);
    std::unique_ptr<float> buf(new float);
    readValues(index, buf.get());
    return *buf;
}

template <>
glm::vec2 VertexAttributeAccessor<glm::vec2>::getInternal(size_t index) const
{
    assert(mAttr->num == 2);
    std::unique_ptr<float[]> buf(new float[2]);
    readValues(index, buf.get());
    return glm::make_vec2(buf.get());
}

template <>
glm::vec3 VertexAttributeAccessor<glm::vec3>::getInternal(size_t index) const
{
    assert(mAttr->num == 3);
    std::unique_ptr<float[]> buf(new float[3]);
    readValues(index, buf.get());
    return glm::make_vec3(buf.get());
}

template <>
glm::vec4 VertexAttributeAccessor<glm::vec4>::getInternal(size_t index) const
{
    assert(mAttr->num == 4);
    std::unique_ptr<float[]> buf(new float[4]);
    readValues(index, buf.get());
    return glm::make_vec4(buf.get());
}

template <>
void VertexAttributeAccessor<float>::setInternal(size_t index, const float& val)
{
    assert(mAttr->num == 1);
    writeValues(index, &val);
}

template <>
void VertexAttributeAccessor<glm::vec2>::setInternal(size_t index, const glm::vec2& val)
{
    assert(mAttr->num == 2);
    writeValues(index, glm::value_ptr(val));
}

template <>
void VertexAttributeAccessor<glm::vec3>::setInternal(size_t index, const glm::vec3& val)
{
    assert(mAttr->num == 3);
    writeValues(index, glm::value_ptr(val));
}

template <>
void VertexAttributeAccessor<glm::vec4>::setInternal(size_t index, const glm::vec4& val)
{
    assert(mAttr->num == 4);
    writeValues(index, glm::value_ptr(val));
}
}
