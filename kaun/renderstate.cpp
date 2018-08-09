#include "renderstate.hpp"
#include "log.hpp"

namespace kaun {
    // These values represent the OpenGL default values
    bool RenderState::currentDepthWrite = true;
    RenderState::DepthFunc RenderState::currentDepthFunc = RenderState::DepthFunc::DISABLED;
	RenderState::FaceDirections RenderState::currentCullFaces = RenderState::FaceDirections::NONE;
	RenderState::FaceOrientation RenderState::currentFrontFace = RenderState::FaceOrientation::CCW;
    bool RenderState::currentBlendEnabled = false;
    RenderState::BlendFactor RenderState::currentBlendSrcFactor = RenderState::BlendFactor::ONE;
    RenderState::BlendFactor RenderState::currentBlendDstFactor = RenderState::BlendFactor::ZERO;
    RenderState::BlendEq RenderState::currentBlendEquation = RenderState::BlendEq::ADD;

    void RenderState::apply(bool force) const {
        if(mDepthWrite != currentDepthWrite || force) {
            glDepthMask(mDepthWrite ? GL_TRUE : GL_FALSE);
            currentDepthWrite = mDepthWrite;
        }

        if(mDepthFunc != currentDepthFunc || force) {
            if(mDepthFunc == DepthFunc::DISABLED) {
                glDisable(GL_DEPTH_TEST);
            } else {
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(static_cast<GLenum>(mDepthFunc));
            }
            currentDepthFunc = mDepthFunc;
        }

        if(mCullFaces != currentCullFaces || force) {
            if(mCullFaces == FaceDirections::NONE) {
                glDisable(GL_CULL_FACE);
            } else {
                glEnable(GL_CULL_FACE);
                glCullFace(static_cast<GLenum>(mCullFaces));
            }
            currentCullFaces = mCullFaces;
        }

        if(mFrontFace != currentFrontFace || force) {
            glFrontFace(static_cast<GLenum>(mFrontFace));
            currentFrontFace = mFrontFace;
        }

        if(mBlendEnabled != currentBlendEnabled || force) {
            if(mBlendEnabled) {
                glEnable(GL_BLEND);
            } else {
                glDisable(GL_BLEND);
            }
            currentBlendEnabled = mBlendEnabled;
        }

        if(mBlendEnabled) {
            if(mBlendSrcFactor != currentBlendSrcFactor || mBlendDstFactor != currentBlendDstFactor || force) {
                glBlendFunc(static_cast<GLenum>(mBlendSrcFactor),
                            static_cast<GLenum>(mBlendDstFactor));
                currentBlendSrcFactor = mBlendSrcFactor;
                currentBlendDstFactor = mBlendDstFactor;
            }
            if(mBlendEquation != currentBlendEquation || force) {
                glBlendEquation(static_cast<GLenum>(mBlendEquation));
                currentBlendEquation = mBlendEquation;
            }
        }
    }
}
