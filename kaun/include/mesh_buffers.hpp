#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <glad/glad.h>

#include "mesh_vertexformat.hpp"

namespace kaun {
    enum class UsageHint : GLenum {
        STATIC = GL_STATIC_DRAW,
        STREAM = GL_STREAM_DRAW,
        DYNAMIC = GL_DYNAMIC_DRAW
    };

    class GLBuffer {
    protected:
        GLenum mTarget;
        int mSize;
        UsageHint mUsage;
        // Actually we don't care about the type, but we can't declare a std::unique_ptr<void>
        // this is because void doesn't have a destructor, making that type incomplete
        using VBODataType = uint8_t;
        std::unique_ptr<VBODataType[]> mData;
        GLuint mBufferObject;
        int mLastUploadedSize;
        int mUploadCount;

    public:
        // GLBuffer does take ownership of the data passed to it!
        GLBuffer(GLenum target, void* data, size_t size, UsageHint usage) :
                mTarget(target), mSize(size), mUsage(usage), mData(reinterpret_cast<VBODataType*>(data)),
                mBufferObject(0), mLastUploadedSize(0), mUploadCount(0) {}

        ~GLBuffer() {
            if(mBufferObject != 0) glDeleteBuffers(1, &mBufferObject);
        }

        GLBuffer(const GLBuffer& other) = delete;
        GLBuffer& operator=(const GLBuffer& other) = delete;

        // http://hacksoflife.blogspot.de/2015/06/glmapbuffer-no-longer-cool.html - Don't implement map()?

        void upload();

        virtual void reallocate(size_t num, bool copyOld) = 0;

        virtual void* getData() {
            return mData.get();
        }

        int getSize() const { return mSize; }

        int getUploadCount() const {return mUploadCount;}

        // If you uploaded your data, you can call release to delete the local copy
        void freeLocal() {
            mData.reset();
        }

        void bind() {
            if(mUploadCount == 0) upload();
            glBindBuffer(mTarget, mBufferObject);
        }

        void unbind() const {
            glBindBuffer(mTarget, 0);
        }
    };


    // VertexBuffer/IndexBuffer represent the vertex data in it.
    // That means that they have full ownership of the data, you may reallocate it or get access to the data they points to
    // but never change the data they point to

    class VertexBuffer : public GLBuffer {
    private:
        const VertexFormat& mVertexFormat;
        size_t mNumVertices;

    public:
        // The vertex formats will be copied, so it's not possible to to dangerous shenanigans by changing them after they were used
        VertexBuffer(const VertexFormat& format, UsageHint usage = UsageHint::STATIC) :
                GLBuffer(GL_ARRAY_BUFFER, nullptr, 0, usage),
                mVertexFormat(format), mNumVertices(0) {}

        VertexBuffer(const VertexFormat& format, size_t numVertices, UsageHint usage = UsageHint::STATIC) :
                GLBuffer(GL_ARRAY_BUFFER, nullptr, 0, usage),
                mVertexFormat(format), mNumVertices(numVertices) {
            reallocate(numVertices);
        }

        // This constructor will *take ownership* of the data pointed to by data
        // If you are using a std::unique_ptr yourself, use std::move
        VertexBuffer(const VertexFormat& format, void* data, size_t numVertices, UsageHint usage = UsageHint::STATIC) :
                GLBuffer(GL_ARRAY_BUFFER, data, format.getStride()*numVertices, usage),
                mVertexFormat(format), mNumVertices(numVertices) {}

        // If nothing has been allocated yet, also call this function
        void reallocate(size_t numVertices, bool copyOld = false);

        size_t getNumVertices() const { return mNumVertices; }
        const VertexFormat& getVertexFormat() const { return mVertexFormat; }
    };


    enum class IndexBufferType : GLenum {
        UI8 = GL_UNSIGNED_BYTE,
        UI16 = GL_UNSIGNED_SHORT,
        UI32 = GL_UNSIGNED_INT
    };

    int getIndexBufferTypeSize(IndexBufferType type);
    IndexBufferType getIndexBufferType(size_t vertexCount);

    class IndexBuffer : public GLBuffer {
    private:
        IndexBufferType mDataType;
        size_t mNumIndices;

    public:
        IndexBuffer(size_t vertexCount, UsageHint usage = UsageHint::STATIC) :
                GLBuffer(GL_ELEMENT_ARRAY_BUFFER, nullptr, 0, usage),
                mNumIndices(0) {
            mDataType = getIndexBufferType(vertexCount);
        }

        IndexBuffer(size_t vertexCount, size_t num, UsageHint usage = UsageHint::STATIC) :
                GLBuffer(GL_ELEMENT_ARRAY_BUFFER, nullptr, 0, usage),
                mNumIndices(num) {
            mDataType = getIndexBufferType(vertexCount);
            reallocate(num);
        }

        IndexBuffer(uint8_t* data, size_t num, UsageHint usage = UsageHint::STATIC) :
                GLBuffer(GL_ELEMENT_ARRAY_BUFFER, data, num*sizeof(uint8_t), usage),
                mDataType(IndexBufferType::UI8), mNumIndices(num) {}
        IndexBuffer(uint16_t* data, size_t num, UsageHint usage = UsageHint::STATIC) :
                GLBuffer(GL_ELEMENT_ARRAY_BUFFER, data, num*sizeof(uint16_t), usage),
                mDataType(IndexBufferType::UI16), mNumIndices(num) {}
        IndexBuffer(uint32_t* data, size_t num, UsageHint usage = UsageHint::STATIC) :
                GLBuffer(GL_ELEMENT_ARRAY_BUFFER, data, num*sizeof(uint32_t), usage),
                mDataType(IndexBufferType::UI32), mNumIndices(num) {}

        size_t getNumIndices() const {return mNumIndices;}
        IndexBufferType getDataType() const {return mDataType;}

        template<typename T>
        T* getData() {
            return reinterpret_cast<T*>(mData.get());
        }

        void reallocate(size_t numIndices, bool copyOld = false);

        uint32_t get(size_t index) const;

        void set(size_t index, uint32_t val);
    };
}
