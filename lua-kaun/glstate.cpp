#include "glstate.hpp"
#include <iostream>

GLint maxTextureUnits = -1;

GLenum textureTargets[numTextureTargets] = {
    GL_TEXTURE_2D,
    GL_TEXTURE_3D,
    GL_TEXTURE_2D_ARRAY,
    GL_TEXTURE_CUBE_MAP
};

GLenum getTextureTargets[numTextureTargets] = {
    GL_TEXTURE_BINDING_2D,
    GL_TEXTURE_BINDING_3D,
    GL_TEXTURE_BINDING_2D_ARRAY,
    GL_TEXTURE_BINDING_CUBE_MAP
};

GLint glGetInteger(GLenum pname) {
    GLint val;
    glGetIntegerv(pname, &val);
    return val;
}

LoveGlState saveLoveGlState() {
    LoveGlState state;

    state.vertexArrayBinding = glGetInteger(GL_VERTEX_ARRAY_BINDING);

    state.arrayBufferBinding = glGetInteger(GL_ARRAY_BUFFER_BINDING);
    state.elementBufferBinding = glGetInteger(GL_ELEMENT_ARRAY_BUFFER_BINDING);

    glGetVertexAttribfv(3, GL_CURRENT_VERTEX_ATTRIB, state.constantColorVertexAttrib);

    if(maxTextureUnits < 0) maxTextureUnits = glGetInteger(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
    // This is purely an optimization and may go very wrong
    if(maxTextureUnits > 8) maxTextureUnits = 8; 

    state.activeTexture = glGetInteger(GL_ACTIVE_TEXTURE);

    for(int t = 0; t < numTextureTargets; ++t) {
        state.boundTextures[t].resize(maxTextureUnits, 0);
    }
    
    for(int u = 0; u < maxTextureUnits; ++u) {
        glActiveTexture(GL_TEXTURE0 + u);
        for(int t = 0; t < numTextureTargets; ++t) {
            state.boundTextures[t][u] = glGetInteger(getTextureTargets[t]);
        }
    }

    state.depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    state.stencilTestEnabled = glIsEnabled(GL_STENCIL_TEST);
    state.scissorTestEnabled = glIsEnabled(GL_SCISSOR_TEST);
    state.cullFaceEnabled = glIsEnabled(GL_CULL_FACE);
    state.framebufferSrgbEnabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);

    state.cullFaceMode = glGetInteger(GL_CULL_FACE_MODE);
    state.frontFace = glGetInteger(GL_FRONT_FACE);

    saveLoveViewportState(state);
    glGetFloatv(GL_POINT_SIZE, &state.pointSize);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &state.depthWriteMask);

    state.drawFramebufferBinding = glGetInteger(GL_DRAW_FRAMEBUFFER_BINDING);
    state.readFramebufferBinding = glGetInteger(GL_READ_FRAMEBUFFER_BINDING);

    state.program = glGetInteger(GL_CURRENT_PROGRAM);

    return state;
}

extern void saveLoveViewportState(LoveGlState& state) {
    glGetIntegerv(GL_VIEWPORT, state.viewport);
    glGetIntegerv(GL_SCISSOR_BOX, state.scissorBox);
}

void glSetEnabled(GLenum cap, GLboolean state) {
    if(state == GL_TRUE) {
        glEnable(cap);
    } else {
        glDisable(cap);
    }
}

bool checkErrors(const char* after) {
    GLenum err;
    bool anyErrors = false;
    while((err = glGetError()) != GL_NO_ERROR) {
        std::printf("GL Error after %s: 0x%X\n", after, err);
        anyErrors = true;
    }
    return anyErrors;
}

void restoreLoveGlState(const LoveGlState& state) {
    glBindVertexArray(state.vertexArrayBinding);

    glBindBuffer(GL_ARRAY_BUFFER, state.arrayBufferBinding);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state.elementBufferBinding);

    glVertexAttrib4fv(3, state.constantColorVertexAttrib);

    for(int u = 0; u < maxTextureUnits; ++u) {
        glActiveTexture(GL_TEXTURE0 + u);
        for(int t = 0; t < numTextureTargets; ++t) {
            glBindTexture(textureTargets[t], state.boundTextures[t][u]);
        }
    }

    glActiveTexture(state.activeTexture);

    glSetEnabled(GL_DEPTH_TEST, state.depthTestEnabled);
    glSetEnabled(GL_STENCIL_TEST, state.stencilTestEnabled);
    glSetEnabled(GL_SCISSOR_TEST, state.scissorTestEnabled);
    glSetEnabled(GL_CULL_FACE, state.cullFaceEnabled);
    glSetEnabled(GL_FRAMEBUFFER_SRGB, state.framebufferSrgbEnabled);

    glCullFace(state.cullFaceMode);
    glFrontFace(state.frontFace);

    restoreLoveViewportState(state);
    glPointSize(state.pointSize);
    glDepthMask(state.depthWriteMask);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, state.drawFramebufferBinding);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, state.readFramebufferBinding);

    glUseProgram(state.program);
}

extern void restoreLoveViewportState(const LoveGlState& state) {
    glViewport(state.viewport[0], state.viewport[1], state.viewport[2], state.viewport[3]);
    glScissor(state.scissorBox[0], state.scissorBox[1], state.scissorBox[2], state.scissorBox[4]);
}
