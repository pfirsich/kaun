#include "renderattachment.hpp"

namespace kaun {
    RenderBuffer::RenderBuffer(PixelFormat format, int width, int height) :
            mRbo(0), mPixelFormat(format), mWidth(width), mHeight(height) {
        glGenRenderbuffers(1, &mRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
        glRenderbufferStorage(GL_RENDERBUFFER, static_cast<GLenum>(format), width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    void RenderBuffer::attach(AttachmentPoint attachment) const {
        glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, static_cast<GLenum>(attachment), GL_RENDERBUFFER, mRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
}
