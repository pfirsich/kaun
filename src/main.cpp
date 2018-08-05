#include <kaun/kaun.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

int main(int argc, char** argv) {
    kaun::init();

    kaun::WindowProperties props;
    props.msaaSamples = 8;
    kaun::setupWindow("Kaun Test", 800, 600, props);

    kaun::setProjection(glm::perspective(90.0f, 600.0f/800.0f, 0.1f, 100.0f));
    glViewport(0, 0, 800, 600);

    kaun::Transform cameraTransform;
    cameraTransform.lookAtPos(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, -1.0f));

    kaun::Shader shader("media/shaders/default.frag", "media/shaders/default.vert");
    kaun::setShader(shader);

    //kaun::Mesh* mesh = kaun::Mesh::box(1.0f, 1.0f, 1.0f, kaun::defaultVertexFormat);
    //kaun::Mesh* mesh = kaun::Mesh::sphere(1.0f, 32, 12, false, kaun::defaultVertexFormat);
    kaun::Transform meshTrafo;

    kaun::Texture tex = kaun::Texture("media/default.png");
    //kaun::Texture* tex = kaun::Texture::checkerBoard(16, 16, 2);
    //kaun::Texture* tex = kaun::Texture::pixel(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));

    // kaun::RenderTarget::backBuffer.setClearColor(0.5f, 0.5f, 0.5f);

    // kaun::setRenderTarget(kaun::RenderTarget::backBuffer); // set by default

    // kaun::setRenderState(...) // default state is set from the start

    kaun::RenderState state;
    //state.setCullFaces(kaun::RenderState::FaceDirections::NONE);
    //state.setFrontFace(kaun::RenderState::FaceOrientation::CCW);

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);

    bool quit = false;
    kaun::closeSignal.connect([&quit]() {quit = true;});
    float lastTime = kaun::getTime();
    while(!quit) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float t = kaun::getTime();
        float dt = t - lastTime;
        lastTime = t;

        meshTrafo.rotate(dt, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 modelMatrix = meshTrafo.getMatrix();
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));
        glm::mat4 viewMatrix = glm::inverse(cameraTransform.getMatrix());

        kaun::Texture::markAllUnitsAvailable();

        state.apply();

        shader.bind();
        shader.setUniform("modelMatrix", modelMatrix);
        shader.setUniform("normalMatrix", normalMatrix);
        shader.setUniform("viewMatrix", viewMatrix);
        shader.setUniform("projectionMatrix", projectionMatrix);
        shader.setUniform("base", tex);

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
