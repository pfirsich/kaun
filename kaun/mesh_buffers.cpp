#include "mesh_buffers.hpp"
#include <cassert>

#include "log.hpp"

namespace kaun {
    void GLBuffer::upload() {
        mUploadCount++;
        if(mBufferObject == 0) {
            glGenBuffers(1, &mBufferObject);
        }
        glBindBuffer(mTarget, mBufferObject);
        if(mLastUploadedSize != mSize) {
            glBufferData(mTarget, mSize, mData.get(), static_cast<GLenum>(mUsage));
            mLastUploadedSize = mSize;
        } else {
            glBufferSubData(mTarget, 0, mSize, mData.get());
        }
        glBindBuffer(mTarget, 0);
    }


    void VertexBuffer::reallocate(size_t numVertices, bool copyOld) {
        size_t newSize = mVertexFormat.getStride()*numVertices;
        std::unique_ptr<VBODataType[]> newData(new VBODataType[newSize]);
        if(mData.get() != nullptr && copyOld)
            memcpy(newData.get(), mData.get(), mSize);
        mData.reset(newData.release());
        mNumVertices = numVertices;
        mSize = newSize;
    }

    int getIndexBufferTypeSize(IndexBufferType type) {
        switch(type) {
            case IndexBufferType::UI8:
                return 1; break;
            case IndexBufferType::UI16:
                return 2; break;
            case IndexBufferType::UI32:
                return 4; break;
        }
        return 0; // This never happens, but g++ whines
    }

    IndexBufferType getIndexBufferType(size_t vertexCount) {
        if(vertexCount < (1 << 8)) {
            return IndexBufferType::UI8;
        } else if(vertexCount < (1 << 16)) {
            return IndexBufferType::UI16;
        } else { // If it's even bigger than 1 << 32, this result will be wrong
            return IndexBufferType::UI32;
        }
    }

    void IndexBuffer::reallocate(size_t numIndices, bool copyOld) {
        size_t newSize = getIndexBufferTypeSize(mDataType)*numIndices;
        std::unique_ptr<VBODataType[]> newData(new VBODataType[newSize]);
        if(mData.get() != nullptr && copyOld)
			memcpy(newData.get(), mData.get(), mSize);
        mData.reset(newData.release());
        mNumIndices = numIndices;
        mSize = newSize;
    }

    uint32_t IndexBuffer::get(size_t index) const {
        switch(mDataType) {
            case IndexBufferType::UI8:
                return static_cast<uint32_t>(*(reinterpret_cast< uint8_t*>(mData.get()) + index));
                break;
            case IndexBufferType::UI16:
                return static_cast<uint32_t>(*(reinterpret_cast<uint16_t*>(mData.get()) + index));
                break;
            case IndexBufferType::UI32:
                return static_cast<uint32_t>(*(reinterpret_cast<uint32_t*>(mData.get()) + index));
                break;
			default:
				assert(false && "Unknown index buffer type");
				return 0;
				break;
        }
    }

    void IndexBuffer::set(size_t index, uint32_t val) {
        switch(mDataType) {
            case IndexBufferType::UI8:
                *(reinterpret_cast< uint8_t*>(mData.get()) + index) = static_cast< uint8_t>(val);
                break;
            case IndexBufferType::UI16:
                *(reinterpret_cast<uint16_t*>(mData.get()) + index) = static_cast<uint16_t>(val);
                break;
            case IndexBufferType::UI32:
                *(reinterpret_cast<uint32_t*>(mData.get()) + index) = static_cast<uint32_t>(val);
                break;
        }
    }
}
