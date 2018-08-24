#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace kaun {
    enum class PixelFormat : GLenum {
        NONE = 0, // for default arguments and such
        R8 = GL_R8,
        RG8 = GL_RG8,
        RGB8 = GL_RGB8,
        RGBA8 = GL_RGBA8,
        SRGB8 = GL_SRGB8,
        SRGB8A8 = GL_SRGB8_ALPHA8,
        R16F = GL_R16F,
        RG16F = GL_RG16F,
        RGB16F = GL_RGB16F,
        RGBA16F = GL_RGBA16F,
        R32F = GL_R32F,
        RG32F = GL_RG32F,
        RGB32F = GL_RGB32F,
        RGBA32F = GL_RGBA32F,
        RGB10_A2 = GL_RGB10_A2,
        RG11F_B10F = GL_R11F_G11F_B10F,
        RGB9E5 = GL_RGB9_E5,
        DEPTH16 = GL_DEPTH_COMPONENT16,
        DEPTH24 = GL_DEPTH_COMPONENT24,
        DEPTH32F = GL_DEPTH_COMPONENT32F,
        DEPTH32F_STENCIL8 = GL_DEPTH32F_STENCIL8,
        DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,
        STENCIL8 = GL_STENCIL_INDEX8
    };

    class RenderAttachment {
    public:
        virtual void attach(GLenum attachmentPoint) const = 0;
        virtual ~RenderAttachment() = default;

        virtual PixelFormat getPixelFormat() const = 0;
        virtual int getWidth() const = 0;
        virtual int getHeight() const = 0;
        virtual size_t getSamples() const = 0;

        glm::ivec2 getDimensions() const { return glm::ivec2(getWidth(), getHeight()); }
        bool hasDepth() const;
        bool hasStencil() const;
        GLbitfield getClearMask() const;
    };

    class RenderBuffer : public RenderAttachment {
    private:
        GLuint mRbo;
        PixelFormat mPixelFormat;
        int mWidth, mHeight;
        size_t mSamples;

    public:
        RenderBuffer(PixelFormat format, int width, int height, size_t samples = 0);
        ~RenderBuffer() { if(mRbo != 0) glDeleteRenderbuffers(1, &mRbo); }

        void attach(GLenum attachmentPoint) const;

        PixelFormat getPixelFormat() const { return mPixelFormat; }
        int getWidth() const { return mWidth; }
        int getHeight() const { return mHeight; }
        size_t getSamples() const { return mSamples; }
    };
}
