#pragma once

#include <vector>

#include "renderattachment.hpp"

namespace kaun {
class RenderTarget {
protected:
    static std::vector<RenderTarget*> renderTargetCache;

    std::vector<const RenderAttachment*> mColorAttachments;
    const RenderAttachment* mDepthStencil;
    GLuint mFbo;
    size_t mSamples;
    int mWidth, mHeight;
    GLbitfield mClearMask;

    void createFbo();

public:
    static const RenderTarget* currentDraw;
    static const RenderTarget* currentRead;
    static RenderTarget* get(const std::vector<const RenderAttachment*>& colorAttachments,
        const RenderAttachment* depthStencil);
    static void ensureGlState();

    RenderTarget(const std::vector<const RenderAttachment*>& colorAttachments,
        const RenderAttachment* depthStencil);

    int getWidth() const
    {
        return mWidth;
    }
    int getHeight() const
    {
        return mHeight;
    }
    glm::ivec2 getDimensions() const
    {
        return glm::ivec2(getWidth(), getHeight());
    }
    GLbitfield getClearMask() const
    {
        return mClearMask;
    }

    void setViewport() const;
    void bind(bool read = true, bool draw = true) const;

protected:
    // this is only used by WindowRenderTarget
    RenderTarget()
        : mColorAttachments()
        , mDepthStencil(nullptr)
        , mFbo(0)
        , mWidth(0)
        , mHeight(0)
        , mSamples(0)
        , mClearMask(0)
    {
    }

public:
    class Window;
};

class RenderTarget::Window : public RenderTarget {
private:
    Window()
        : RenderTarget()
    {
        // I don't think it's possible to get the number of multisamples and the kind of attachment
        // for the default FBO (0) but mClearMask = all, mSamples = 0 works for now
        mClearMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
    }
    Window(const Window& other) = delete;
    Window& operator=(const Window& other) = delete;

public:
    static Window* instance()
    {
        static Window _instance;
        return &_instance;
    }

    // If I relied on the SDL dependency, I would just SDL_GL_GetDrawableSize on every bind
    void setDimensions(int width, int height)
    {
        mWidth = width;
        mHeight = height;
    }
};

void setWindowDimensions(int width, int height);

RenderTarget* setRenderTarget(const std::vector<const RenderAttachment*>& colorAttachments,
    const RenderAttachment* depthStencil, bool blitCurrent);
RenderTarget* setRenderTarget(const RenderTarget& renderTarget, bool blitCurrent);
}
