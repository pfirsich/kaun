#pragma once

#include <glad/glad.h>

namespace kaun {
    enum class PixelFormat : GLenum {
        NONE = 0, // for default arguments and such
        R = GL_R8,
        R16F = GL_R16F,
        RG = GL_RG8,
        RG16F = GL_RG16F,
        RGB = GL_RGB8,
        RGB16F = GL_RGB16F,
        RGBA = GL_RGBA8,
        RGBA16F = GL_RGBA16F,
        RGB10_A2 = GL_RGB10_A2,
        RG11F_B10F = GL_R11F_G11F_B10F,
        RGBE = GL_RGB9_E5,
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

        virtual PixelFormat getPixelFormat() const = 0;
        virtual int getWidth() const = 0;
        virtual int getHeight() const = 0;
        glm::ivec2 getDimensions() const { return glm::ivec2(getWidth(), getHeight()); }
    };

    class RenderBuffer : public RenderAttachment {
    private:
        GLuint mRbo;
        PixelFormat mPixelFormat;
        int mWidth, mHeight;

    public:
        RenderBuffer(PixelFormat format, int width, int height);
        ~RenderBuffer() { if(mRbo != 0) glDeleteRenderbuffers(1, &mRbo); }

        void attach(GLenum attachmentPoint) const;

        PixelFormat getPixelFormat() const { return mPixelFormat; }
        int getWidth() const { return mWidth; }
        int getHeight() const { return mHeight; }
    };
}
