#include "renderstate.hpp"
#include "log.hpp"

namespace kaun {
    RenderState RenderState::currentState = glDefaults();

    void RenderState::ensureGlState() {
        currentState.apply(true);
    }

    // OpenGL default vlaues (i.e. the initial values of these states at the start of the program)
    RenderState RenderState::glDefaults() {
        RenderState state;
        state.mDepthWrite = true;
        state.mDepthFunc = RenderState::DepthFunc::DISABLED;
        state.mCullFaces = RenderState::FaceDirections::NONE;
        state.mFrontFace = RenderState::FaceOrientation::CCW;
        state.mBlendEnabled = false;
        state.mBlendSrcFactor = RenderState::BlendFactor::ONE;
        state.mBlendDstFactor = RenderState::BlendFactor::ZERO;
        state.mBlendEquation = RenderState::BlendEq::ADD;
        return state;
    }

    void RenderState::apply(bool force) const {
        if(mDepthWrite != currentState.mDepthWrite || force) {
            glDepthMask(mDepthWrite ? GL_TRUE : GL_FALSE);
            currentState.mDepthWrite = mDepthWrite;
        }

        if(mDepthFunc != currentState.mDepthFunc || force) {
            if(mDepthFunc == DepthFunc::DISABLED) {
                glDisable(GL_DEPTH_TEST);
            } else {
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(static_cast<GLenum>(mDepthFunc));
            }
            currentState.mDepthFunc = mDepthFunc;
        }

        if(mCullFaces != currentState.mCullFaces || force) {
            if(mCullFaces == FaceDirections::NONE) {
                glDisable(GL_CULL_FACE);
            } else {
                glEnable(GL_CULL_FACE);
                glCullFace(static_cast<GLenum>(mCullFaces));
            }
            currentState.mCullFaces = mCullFaces;
        }

        if(mFrontFace != currentState.mFrontFace || force) {
            glFrontFace(static_cast<GLenum>(mFrontFace));
            currentState.mFrontFace = mFrontFace;
        }

        if(mBlendEnabled != currentState.mBlendEnabled || force) {
            if(mBlendEnabled) {
                glEnable(GL_BLEND);
            } else {
                glDisable(GL_BLEND);
            }
            currentState.mBlendEnabled = mBlendEnabled;
        }

        if(mBlendEnabled) {
            if(mBlendSrcFactor != currentState.mBlendSrcFactor
                    || mBlendDstFactor != currentState.mBlendDstFactor
                    || force) {
                glBlendFunc(static_cast<GLenum>(mBlendSrcFactor),
                            static_cast<GLenum>(mBlendDstFactor));
                currentState.mBlendSrcFactor = mBlendSrcFactor;
                currentState.mBlendDstFactor = mBlendDstFactor;
            }
            if(mBlendEquation != currentState.mBlendEquation || force) {
                glBlendEquation(static_cast<GLenum>(mBlendEquation));
                currentState.mBlendEquation = mBlendEquation;
            }
        }
    }
}
