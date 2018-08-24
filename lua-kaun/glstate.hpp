#pragma once

#include <vector>

#include <glad/glad.h>

const int numTextureTargets = 4;

struct LoveGlState {
    // glBindBuffer
    GLuint arrayBufferBinding; // GL_ARRAY_BUFFER
    GLuint elementBufferBinding; // GL_ELEMENT_ARRAY_BUFFER

    // glActive Texture
    GLenum activeTexture;

    // glBindTexture
    // index is target, vector index is texture unit
    std::vector<GLuint> boundTextures[numTextureTargets];

    // glEnable
    GLboolean depthTestEnabled; // GL_DEPTH_TEST
    GLboolean stencilTestEnabled; // GL_STENCIL_TEST
    GLboolean scissorTestEnabled; // GL_SCISSOR_TEST
    GLboolean cullFaceEnabled; // GL_CULL_FACE
    GLboolean framebufferSrgbEnabled; // GL_FRAMEBUFFER_SRGB
    GLboolean blendEnabled; // GL_BLEND

    // glCullFace
    GLenum cullFaceMode;
    GLenum frontFace;

    // glBindVertexArray
    GLuint vertexArrayBinding;
    // maybe the vertex array object alone is not enough and we need to save the following:
    // glEnableVertexAttribArray
    //     0 - 31
    // glVertexAttribDivisor
    //     0 - 31 (0 or 1)
    // glVertexAttribPointer?

    // glVertexAttrib4f for attrib ATTRIB_CONSTANTCOLOR = 3
    GLfloat constantColorVertexAttrib[4];

    // glViewport
    GLint viewport[4];

    // glScissor
    GLint scissorBox[4];

    // glPointSize
    GLfloat pointSize;

    // glDepthMask
    GLboolean depthWriteMask;

    // glBindFramebuffer
    GLuint drawFramebufferBinding; // GL_DRAW_FRAMEBUFFER
    GLuint readFramebufferBinding; // GL_READ_FRAMEBUFFER

    // glUseProgram
    GLuint program;

    // glBlendEquationSeparate
    GLenum blendEquationRgb;
    GLenum blendEquationAlpha;

    // glBlendFuncSeparate
    GLenum blendSrcRgb;
    GLenum blendSrcAlpha;
    GLenum blendDstRgb;
    GLenum blendDstAlpha;

    // GL_BLEND, glBlendEquation, glBlendFunc
    // glStencilFunc, glStencilOp, glStencilMask
};

LoveGlState saveLoveGlState();
void restoreLoveGlState(const LoveGlState& state);

void saveLoveViewportState(LoveGlState& state);
void restoreLoveViewportState(const LoveGlState& state);
