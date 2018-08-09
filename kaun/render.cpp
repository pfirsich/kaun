#include <vector>

#include <glm/glm.hpp>

#include "render.hpp"

namespace kaun {
    glm::ivec4 viewport;

    GLenum colorAttachmentPoints[8] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
        GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7};

    std::unordered_map<GLenum, std::string> framebufferStatus = {
        {GL_FRAMEBUFFER_UNDEFINED, "undefined"},
        {GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT, "incomplete_attachment"},
        {GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT, "incomplete_missing_attachment"},
        {GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER, "incomplete_draw_buffer"},
        {GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER, "incomplete_read_buffer"},
        {GL_FRAMEBUFFER_UNSUPPORTED, "unsupported"},
        {GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE, "incomplete_multisample"},
        {GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE, "incomplete_multisample"},
        {GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS, "incomplete_layer_targets"}
    };

    GLenum getDepthStencilAttachmentPoint(const RenderAttachment* attachment) {
        switch(attachment->getPixelFormat()) {
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

    GLuint createFBO(const std::vector<const RenderAttachment*>& colorAttachments,
                     const RenderAttachment* depthStencil) {
        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        for(size_t i = 0; i < colorAttachments.size(); ++i) {
            colorAttachments[i]->attach(GL_COLOR_ATTACHMENT0 + i);
        }
        if(depthStencil) {
            GLenum attachmentPoint = getDepthStencilAttachmentPoint(depthStencil);
            assert(attachmentPoint > 0);
            depthStencil->attach(attachmentPoint);
        }

        if(colorAttachments.size() == 0) { // depth/stencil only
            glReadBuffer(GL_NONE);
            glDrawBuffer(GL_NONE);
        } else {
            glDrawBuffers(colorAttachments.size(), colorAttachmentPoints);
        }

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE) {
            LOG_ERROR("Framebuffer object %d is incomplete after initialization!: %s", fbo,
                framebufferStatus[status].c_str());
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return fbo;
    }

    struct fboCacheEntry {
        std::vector<const RenderAttachment*> color;
        const RenderAttachment* depthStencil;
        GLuint fbo;
        int width, height;

        fboCacheEntry(const std::vector<const RenderAttachment*>& colorAttachments,
            const RenderAttachment* depthStencil, int width, int height) :
            color(colorAttachments), depthStencil(depthStencil), fbo(0), width(width), height(height) {
            fbo = createFBO(colorAttachments, depthStencil);
        }
    };
    std::vector<fboCacheEntry> fboCache;

    fboCacheEntry* getFboCacheEntry(const std::vector<const RenderAttachment*>& colorAttachments,
                                    const RenderAttachment* depthStencil) {
        for(auto& entry : fboCache) {
            if(entry.depthStencil != depthStencil) continue;
            if(entry.color.size() == colorAttachments.size()) {
                bool match = true;
                for(size_t i = 0; i < colorAttachments.size(); ++i) {
                    if(entry.color[i] != colorAttachments[i]) {
                        match = false;
                        break;
                    }
                }
                if(match) return &entry;
            }
        }
        return nullptr;
    }

    fboCacheEntry* addFboCacheEntry(const std::vector<const RenderAttachment*>& colorAttachments,
                                    const RenderAttachment* depthStencil) {
        // make sure all attachments are the same size and determine joint size
        int width, height;
        if(colorAttachments.size() > 0) {
            width = colorAttachments[0]->getWidth();
            height = colorAttachments[0]->getHeight();

            bool mismatch = false;
            for(auto attachment : colorAttachments) {
                if(width != attachment->getWidth() || height != attachment->getHeight()) {
                    mismatch = true;
                    break;
                }
            }

            if(depthStencil) {
                if(width != depthStencil->getWidth() || height != depthStencil->getHeight()) {
                    mismatch = true;
                }
                GLenum attachmentPoint = getDepthStencilAttachmentPoint(depthStencil);
                if(attachmentPoint == 0) {
                    LOG_ERROR("Invalid pixel format for depth/stencil attachment!");
                    return nullptr;
                }
            }

            if(mismatch) {
                LOG_ERROR("Attachments are of different sizes!");
                return nullptr;
            }
        } else {
            // colorAttachments.size() == 0, depthStencil != nullptr
            width = depthStencil->getWidth();
            height = depthStencil->getHeight();
        }
        fboCache.emplace_back(colorAttachments, depthStencil, width, height);
        return &fboCache.back();
    }

    void setRenderTarget(const std::vector<const RenderAttachment*>& colorAttachments,
                         const RenderAttachment* depthStencil) {
        if(colorAttachments.size() == 0 && depthStencil == nullptr) {
            // bind backbuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
        } else {
            fboCacheEntry* entry = getFboCacheEntry(colorAttachments, depthStencil);
            if(!entry) {
                entry = addFboCacheEntry(colorAttachments, depthStencil);
            }
            if(entry) {
                glBindFramebuffer(GL_FRAMEBUFFER, entry->fbo);
                glViewport(0, 0, entry->width, entry->height);
            }
        }
    }

    void clear(const glm::vec4& color, int colorAttachmentIndex) {
        glClearBufferfv(GL_COLOR, colorAttachmentIndex, glm::value_ptr(color));
    }

    void clearDepth(float value) {
        glClearBufferfv(GL_DEPTH, 0, &value);
    }

    void setViewport(int x, int y, int w, int h) {
        viewport = glm::ivec4(x, y, w, h);
        glViewport(x, y, w, h);
    }

    void setViewport(const glm::ivec4& vp) {
        setViewport(vp.x, vp.y, vp.z, vp.w);
    }

    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 modelMatrix;
    glm::mat3 normalMatrix;

    struct RenderQueueEntry {
        Mesh* mesh;
        Shader* shader;
        std::vector<Uniform> uniforms;
        RenderState renderState;

        RenderQueueEntry(Mesh* mesh, Shader* shader, const RenderState& renderState) :
                mesh(mesh), shader(shader), renderState(renderState) {}
    };

    std::vector<RenderQueueEntry> renderQueue;

    void setProjection(const glm::mat4& matrix) {
        projectionMatrix = matrix;
    }

    void setViewMatrix(const glm::mat4 view) {
        viewMatrix = view;
    }

    void setViewTransform(const Transform& viewTransform) {
        viewMatrix = glm::inverse(viewTransform.getMatrix());
    }

    void setModelMatrix(const glm::mat4& model) {
        modelMatrix = model;
        normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    }

    void setModelTransform(const Transform& modelTransform) {
        setModelMatrix(modelTransform.getMatrix());
    }

    void draw(Mesh& mesh, Shader& shader, const std::vector<Uniform>& uniforms, const RenderState& state) {
		renderQueue.emplace_back(&mesh, &shader, state);
		RenderQueueEntry& entry = renderQueue.back();
        entry.uniforms = uniforms;
        //entry.uniforms.emplace_back("kaun_viewport", viewport);
        entry.uniforms.emplace_back("kaun_view", viewMatrix);
        //entry.uniforms.emplace_back("kaun_invView", );
        entry.uniforms.emplace_back("kaun_projection", projectionMatrix);
        //entry.uniforms.emplace_back("kaun_invProjection", );
        //entry.uniforms.emplace_back("kaun_viewProjection", );
        //entry.uniforms.emplace_back("kaun_invViewProjection", );
        entry.uniforms.emplace_back("kaun_model", modelMatrix);
        entry.uniforms.emplace_back("kaun_normal", normalMatrix);
        //entry.uniforms.emplace_back("kaun_modelView", );
        //entry.uniforms.emplace_back("kaun_modelViewProjection", );
    }

    void flush() {
        // sort here!
        for(auto& entry : renderQueue) {
            entry.renderState.apply();
            entry.shader->bind();
            Texture::markAllUnitsAvailable();
            for(auto& uniform : entry.uniforms) {
                Shader::UniformLocation loc = entry.shader->getUniformLocation(uniform.getName());
                if(loc != -1) uniform.set(loc);
            }
            entry.mesh->draw();
        }
        renderQueue.clear();
    }
}
