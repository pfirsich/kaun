#include <kaun/kaun.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

int main(int argc, char** argv) {
    kaun::init();

    kaun::setupWindow("Kaun Test", 800, 600);

    kaun::setProjection(glm::perspective(90.0f, 800.0f/600.0f, 0.1f, 100.0f));
    glViewport(0, 0, 800, 600);

    kaun::Transform cameraTransform;
    cameraTransform.lookAtPos(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, -1.0f));

    kaun::Shader shader("media/shaders/default.frag", "media/shaders/default.vert");

    kaun::Mesh* mesh = kaun::boxMesh(1.0f, 1.0f, 1.0f, kaun::defaultVertexFormat);

    // kaun::RenderTarget::backBuffer.setClearColor(0.5f, 0.5f, 0.5f);

    // kaun::setRenderTarget(kaun::RenderTarget::backBuffer); // set by default

    // kaun::setRenderState(...) // default state is set from the start

    kaun::setShader(shader);

    kaun::Transform meshTrafo;

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);

    kaun::RenderState state;
    state.setCullFaces(kaun::RenderState::FaceDirections::NONE);

    bool quit = false;
    kaun::closeSignal.connect([&quit]() {quit = true;});
    while(!quit) {
        glClear(GL_COLOR_BUFFER_BIT);

        meshTrafo.rotate(1e-3f, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 modelMatrix = meshTrafo.getMatrix();
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));
        glm::mat4 viewMatrix = glm::inverse(cameraTransform.getMatrix());

        state.apply();

        shader.bind();
        shader.setUniform("modelMatrix", modelMatrix);
        shader.setUniform("normalMatrix", normalMatrix);
        shader.setUniform("viewMatrix", viewMatrix);
        shader.setUniform("projectionMatrix", projectionMatrix);

        mesh->draw();

        //kaun::setViewTransform(cameraTransform);

        // kaun::setUniform("modelmatrix", kaun::Transform(glm::vec3(-1.0f, 0.0f, 0.0f)));
        // kaun::setUniform("color", kaun::Transform(glm::vec3(1.0f, 0.0f, 0.0f)));
        // kaun::draw(cube);

        // kaun::setUniform("modelmatrix", kaun::Transform(glm::vec3(1.0f, 0.0f, 0.0f)));
        // kaun::setUniform("color", kaun::Transform(glm::vec3(0.0f, 0.0f, 1.0f)));
        // kaun::draw(cube);

        kaun::flush(); // optional

        kaun::updateAndSwap();
    }
    kaun::cleanup();

    return 0;
}
