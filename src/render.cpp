#include <vector>

#include <glm/glm.hpp>

#include "render.hpp"

namespace kaun {
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 modelMatrix;

    Shader* currentShader;
    //RenderTarget* currentRenderTarget;
    RenderState currentRenderState;

    struct RenderQueueEntry {
        Shader* shader;
        //std::vector<Uniform> uniforms;
        Mesh* mesh;
        RenderState renderState;

        RenderQueueEntry(Shader* shader, Mesh* mesh, const RenderState& renderState) :
                shader(shader), mesh(mesh), renderState(renderState) {}
    };

    std::vector<RenderQueueEntry> renderQueue;

    void setProjection(const glm::mat4& matrix) {
        projectionMatrix = matrix;
    }

    void setViewTransform(const Transform& viewTransform) {
        viewMatrix = glm::inverse(viewTransform.getMatrix());
    }

    void setModelTransform(const Transform& modelTransform) {
        modelMatrix = modelTransform.getMatrix();
    }

    //void setRenderTarget(RenderTarget& renderTarget) {
    //    currentRenderTarget = &renderTarget;
    //}

    RenderState& getRenderState() {
        return currentRenderState;
    }

    void setRenderstate(const RenderState& state) {
        currentRenderState = state;
    }

    void setShader(Shader& shader) {
        currentShader = &shader;
    }

    void draw(Mesh& mesh) {
		renderQueue.emplace_back(currentShader, &mesh, currentRenderState);
		RenderQueueEntry& entry = renderQueue.back();
        // set up entry.uniforms
        //mesh.setMutable(false);
    }

    void flush() {

    }
}
