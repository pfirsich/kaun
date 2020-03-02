#include "rendertarget.hpp"
#include "render.hpp"

namespace kaun {
std::vector<RenderTarget*> RenderTarget::renderTargetCache;
const RenderTarget* RenderTarget::currentDraw;
const RenderTarget* RenderTarget::currentRead;

const size_t maxColorAttachments = 8;
GLenum colorAttachmentPoints[maxColorAttachments]
    = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
          GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };

std::unordered_map<GLenum, std::string> framebufferStatus
    = { { GL_FRAMEBUFFER_UNDEFINED, "undefined" },
          { GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT, "incomplete_attachment" },
          { GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT, "incomplete_missing_attachment" },
          { GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER, "incomplete_draw_buffer" },
          { GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER, "incomplete_read_buffer" },
          { GL_FRAMEBUFFER_UNSUPPORTED, "unsupported" },
          { GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE, "incomplete_multisample" },
          { GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE, "incomplete_multisample" },
          { GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS, "incomplete_layer_targets" } };

GLenum getDepthStencilAttachmentPoint(const RenderAttachment* attachment)
{
    switch (attachment->getPixelFormat()) {
    case PixelFormat::DEPTH16:
    case PixelFormat::DEPTH24:
    case PixelFormat::DEPTH32F:
        return GL_DEPTH_ATTACHMENT;
    case PixelFormat::DEPTH24_STENCIL8:
    case PixelFormat::DEPTH32F_STENCIL8:
        return GL_DEPTH_STENCIL_ATTACHMENT;
    case PixelFormat::STENCIL8:
        return GL_STENCIL_ATTACHMENT;
    default:
        return 0;
    }
}

void RenderTarget::ensureGlState()
{
    currentDraw->bind(false, true);
    currentRead->bind(true, false);
}

RenderTarget* RenderTarget::get(const std::vector<const RenderAttachment*>& colorAttachments,
    const RenderAttachment* depthStencil)
{
    if (colorAttachments.size() == 0 && depthStencil == nullptr)
        return RenderTarget::Window::instance();

    for (auto entry : renderTargetCache) {
        if (entry->mDepthStencil != depthStencil)
            continue;
        if (entry->mColorAttachments.size() == colorAttachments.size()) {
            bool match = true;
            for (size_t i = 0; i < colorAttachments.size(); ++i) {
                if (entry->mColorAttachments[i] != colorAttachments[i]) {
                    match = false;
                    break;
                }
            }
            if (match)
                return entry;
        }
    }

    RenderTarget* target = new RenderTarget(colorAttachments, depthStencil);
    renderTargetCache.push_back(target);
    return target;
}

void RenderTarget::createFbo()
{
    assert(mFbo == 0);

    glGenFramebuffers(1, &mFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, mFbo);

    for (size_t i = 0; i < mColorAttachments.size(); ++i) {
        mColorAttachments[i]->attach(GL_COLOR_ATTACHMENT0 + i);
    }
    if (mDepthStencil) {
        GLenum attachmentPoint = getDepthStencilAttachmentPoint(mDepthStencil);
        assert(attachmentPoint > 0);
        mDepthStencil->attach(attachmentPoint);
    }

    if (mColorAttachments.size() == 0) { // depth/stencil only
        glReadBuffer(GL_NONE);
        glDrawBuffer(GL_NONE);
    } else {
        assert(mColorAttachments.size() <= maxColorAttachments);
        glDrawBuffers(mColorAttachments.size(), colorAttachmentPoints);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Framebuffer object %d is incomplete after initialization!: %s", mFbo,
            framebufferStatus[status].c_str());
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RenderTarget::RenderTarget(const std::vector<const RenderAttachment*>& colorAttachments,
    const RenderAttachment* depthStencil)
    : mColorAttachments(colorAttachments)
    , mDepthStencil(depthStencil)
    , mFbo(0)
    , mWidth(0)
    , mHeight(0)
    , mSamples(0)
    , mClearMask(0)
{
    // make sure all attachments have the same size/msaa samples and determine the values
    if (colorAttachments.size() > 0) {
        mWidth = colorAttachments[0]->getWidth();
        mHeight = colorAttachments[0]->getHeight();
        mSamples = colorAttachments[0]->getSamples();
        mClearMask = colorAttachments[0]->getClearMask();

        bool sizeMismatch = false;
        bool msaaMismatch = false;
        for (auto attachment : colorAttachments) {
            if (mWidth != attachment->getWidth() || mHeight != attachment->getHeight()) {
                sizeMismatch = true;
                break;
            }

            if (mSamples != attachment->getSamples()) {
                msaaMismatch = true;
                break;
            }
        }

        if (depthStencil) {
            if (mWidth != depthStencil->getWidth() || mHeight != depthStencil->getHeight()) {
                sizeMismatch = true;
            }

            GLenum attachmentPoint = getDepthStencilAttachmentPoint(depthStencil);
            if (attachmentPoint == 0) {
                LOG_ERROR("Invalid pixel format for depth/stencil attachment!");
                return;
            }

            mClearMask |= depthStencil->getClearMask();
        }

        if (sizeMismatch) {
            LOG_ERROR("Attachments are of different sizes!");
            return;
        }

        if (msaaMismatch) {
            LOG_ERROR("Attachments have different number of msaa samples!");
            return;
        }
    } else {
        // colorAttachments.size() == 0, depthStencil != nullptr
        mWidth = depthStencil->getWidth();
        mHeight = depthStencil->getHeight();
        mSamples = depthStencil->getSamples();
        mClearMask = depthStencil->getClearMask();
    }

    createFbo();
}

void RenderTarget::setViewport() const
{
    kaun::setViewport(0, 0, getWidth(), getHeight());
}

void RenderTarget::bind(bool read, bool draw) const
{
    GLenum target = GL_FRAMEBUFFER; // assume (read && write)
    if (read && !draw)
        target = GL_READ_FRAMEBUFFER;
    if (draw && !read)
        target = GL_DRAW_FRAMEBUFFER;
    glBindFramebuffer(target, mFbo);
    if (draw) {
        RenderTarget::currentDraw = this;
        setViewport();
    }
    if (read) {
        RenderTarget::currentRead = this;
    }
}

RenderTarget* setRenderTarget(const std::vector<const RenderAttachment*>& colorAttachments,
    const RenderAttachment* depthStencil, bool blitCurrent)
{
    flush();

    RenderTarget* target = RenderTarget::get(colorAttachments, depthStencil);

    if (blitCurrent) {
        target->bind(false, true); // bind for drawing
        glm::ivec2 currentSize = RenderTarget::currentRead->getDimensions();
        glm::ivec2 size = target->getDimensions();
        GLbitfield commonMask = RenderTarget::currentRead->getClearMask() & target->getClearMask();
        // blit depth/stencil and color separately and turn off GL_FRAMEBUFFER_SRGB before
        // see here:
        // https://devtalk.nvidia.com/default/topic/1038547/opengl/depth-blit-using-glblitframebuffer-yields-wrong-results-when-gl_framebuffer_srgb-is-enabled/
        if (commonMask & GL_COLOR_BUFFER_BIT) {
            glBlitFramebuffer(0, 0, currentSize.x, currentSize.y, 0, 0, size.x, size.y,
                GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
        if (commonMask != GL_COLOR_BUFFER_BIT) {
            // clear everything except color
            GLbitfield mask = commonMask & ~GL_COLOR_BUFFER_BIT;
            bool enabled = getSrgbEnabled();
            if (enabled)
                setSrgbEnabled(false);
            glBlitFramebuffer(
                0, 0, currentSize.x, currentSize.y, 0, 0, size.x, size.y, mask, GL_NEAREST);
            if (enabled)
                setSrgbEnabled(true);
        }
    }

    target->bind(true, true); // bind for both reading and writing

    return target;
}

void setWindowDimensions(int width, int height)
{
    RenderTarget::Window::instance()->setDimensions(width, height);
}
}
