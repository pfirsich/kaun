#pragma once

#include <type_traits>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "mesh_vertexformat.hpp"
#include "mesh_buffers.hpp"

namespace kaun {
    template<typename T>
    class VertexAttributeAccessor {
        static_assert(std::is_same<T, float>::value
                      || std::is_same<T, glm::vec2>::value
                      || std::is_same<T, glm::vec3>::value
                      || std::is_same<T, glm::vec4>::value,
                      "Invalid Type for VertexAttributeAccessor");
    private:
        void* mData;
        int mSize;
        int mStride;
        size_t mCount;
        // can I make this a reference? What happens if the vertex format doesn't have the attribute?
        const VertexAttribute* mAttr;

        template <typename pT>
        const pT* getPointer(int index) const {
            return reinterpret_cast<pT*>(reinterpret_cast<uint8_t*>(mData) + mStride * index + mAttr->offset);
        }

        template <typename pT>
        pT* getPointer(int index) {
            return reinterpret_cast<pT*>(reinterpret_cast<uint8_t*>(mData) + mStride * index + mAttr->offset);
        }

        template <typename pT> void readIntegers(int index, float* buf) const;
        void read2101010(uint32_t val, float* buf, bool isSigned) const;
        void readValues(size_t index, float* buf) const;

        template <typename pT> void writeIntegers(int index, const float* buf);
        void write2101010(uint32_t* p, const float* buf, bool isSigned);
        void writeValues(size_t index, const float* vals);

        T getInternal(size_t index) const;
        void setInternal(size_t index, const T& val);

    public:
        // This is an invalid VertexAttributeAccessor. This only exists so I can return invalid accessors from Mesh::getAccessor
        VertexAttributeAccessor() :
                mData(nullptr), mSize(0), mStride(0), mCount(0), mAttr(nullptr) {
            LOG_ERROR("Created invalid VertexAttributeAccessor!");
        }

        VertexAttributeAccessor(VertexBuffer& buffer, AttributeType attrType) :
                mData(nullptr), mSize(0), mStride(0), mCount(0) {
            auto& format = buffer.getVertexFormat();
            mAttr = format.getAttribute(attrType);
            if(mAttr != nullptr) {
                mData = buffer.getData();
                mSize = buffer.getSize();
                mCount = buffer.getNumVertices();
                mStride = format.getStride();
            } else {
                LOG_ERROR("Attempt to construct VertexAttributeAccessor for non-existent vertex attribute.");
            }
        }

        bool isValid() const { return mData != nullptr; }
        size_t getCount() const { return mCount; }

        T get(int index) const {
            assert(isValid());
            return getInternal(index);
        }

        void set(int index, const T& val) {
            assert(isValid());
            return setInternal(index, val);
        }
    };

    template <typename T>
    template <typename pT>
    void VertexAttributeAccessor<T>::readIntegers(int index, float* buf) const {
        const pT* p = getPointer<pT>(index);
        for(int i = 0; i < mAttr->num; ++i) {
            pT v = p[i];
            if(mAttr->normalized) {
                pT min = std::numeric_limits<pT>::min();
                pT max = std::numeric_limits<pT>::max();
                buf[i] = static_cast<float>(v - min) / (max - min);
                if(std::numeric_limits<pT>::is_signed) {
                    buf[i] = -1.0f + 2.0f * buf[i];
                }
            } else {
                buf[i] = static_cast<float>(v);
            }
        }
    }

    template <typename T>
    template <typename pT>
    void VertexAttributeAccessor<T>::writeIntegers(int index, const float* buf) {
        pT* p = getPointer<pT>(index);
        for(int i = 0; i < mAttr->num; ++i) {
            float v = buf[i];
            if(mAttr->normalized) {
                pT min = std::numeric_limits<pT>::min();
                pT max = std::numeric_limits<pT>::max();
                if(std::numeric_limits<pT>::is_signed) {
                    v = glm::clamp(v, -1.0f, 1.0f);
                    v = v * 0.5f + 0.5f;
                }
                v = glm::clamp(v, 0.0f, 1.0f);
                p[i] = static_cast<pT>(min + v * (max - min));
            } else {
                p[i] = static_cast<pT>(v);
            }
        }
    }

	template<typename T>
    void VertexAttributeAccessor<T>::read2101010(uint32_t val, float* buf, bool isSigned) const {
        assert(false && "Unimplemented");
    }

    template<typename T>
    void VertexAttributeAccessor<T>::readValues(size_t index, float* buf) const {
        switch(mAttr->dataType) {
            case AttributeDataType::I8:
                readIntegers<int8_t>(index, buf);
                break;
            case AttributeDataType::UI8:
                readIntegers<uint8_t>(index, buf);
                break;
            case AttributeDataType::I16:
                readIntegers<int16_t>(index, buf);
                break;
            case AttributeDataType::UI16:
                readIntegers<uint16_t>(index, buf);
                break;
            case AttributeDataType::I32:
                readIntegers<int32_t>(index, buf);
                break;
            case AttributeDataType::UI32:
                readIntegers<uint32_t>(index, buf);
                break;
            case AttributeDataType::F32:
                memcpy(buf, getPointer<float>(index), mAttr->num * sizeof(float));
                break;
            case AttributeDataType::I2_10_10_10:
                read2101010(*getPointer<uint32_t>(index), buf, true);
                break;
            case AttributeDataType::UI2_10_10_10:
                read2101010(*getPointer<uint32_t>(index), buf, false);
                break;
        }
    }

    template<typename T>
    void VertexAttributeAccessor<T>::write2101010(uint32_t* p, const float* buf, bool isSigned) {
        assert(false && "Unimplemented");
    }

    template<typename T>
    void VertexAttributeAccessor<T>::writeValues(size_t index, const float* vals) {
        switch(mAttr->dataType) {
            case AttributeDataType::I8:
                writeIntegers<int8_t>(index, vals);
                break;
            case AttributeDataType::UI8:
                writeIntegers<uint8_t>(index, vals);
                break;
            case AttributeDataType::I16:
                writeIntegers<int16_t>(index, vals);
                break;
            case AttributeDataType::UI16:
                writeIntegers<uint16_t>(index, vals);
                break;
            case AttributeDataType::I32:
                writeIntegers<int32_t>(index, vals);
                break;
            case AttributeDataType::UI32:
                writeIntegers<uint32_t>(index, vals);
                break;
            case AttributeDataType::F32:
                memcpy(getPointer<float>(index), vals, mAttr->num * sizeof(float));
                break;
            case AttributeDataType::I2_10_10_10:
                write2101010(getPointer<uint32_t>(index), vals, true);
                break;
            case AttributeDataType::UI2_10_10_10:
                write2101010(getPointer<uint32_t>(index), vals, false);
                break;
        }
    }
}
