#include "renderattachment.hpp"

namespace kaun {
    RenderBuffer::RenderBuffer(PixelFormat format, int width, int height, size_t samples) :
            mRbo(0), mPixelFormat(format), mWidth(width), mHeight(height), mSamples(samples) {
        glGenRenderbuffers(1, &mRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
        if(samples > 0) {
            // only samples = 0 results in 0 samples according to 4.4 spec
            // everything above, including 1, may give AT LEAST that number of samples
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                                             static_cast<GLenum>(format), width, height);
        } else {
            glRenderbufferStorage(GL_RENDERBUFFER, static_cast<GLenum>(format), width, height);
        }
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    void RenderBuffer::attach(GLenum attachment) const {
        glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, static_cast<GLenum>(attachment), GL_RENDERBUFFER, mRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
}
