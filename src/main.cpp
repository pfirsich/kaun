#include <kaun/kaun.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

int main(int argc, char** argv) {
    kaun::init();

    kaun::WindowProperties props;
    props.msaaSamples = 8;
    kaun::setupWindow("Kaun Test", 800, 600, props);

    glm::mat4 projectionMatrix;
    kaun::resizeSignal.connect([&projectionMatrix](int w, int h) {
        projectionMatrix = glm::perspective(glm::radians(45.0f),
            static_cast<float>(w)/h, 0.1f, 100.0f);
        kaun::setProjection(projectionMatrix);
        kaun::setViewport(0, 0, w, h);
	});
    auto size = kaun::getWindowSize();
    kaun::resizeSignal.emit(size.x, size.y);

    kaun::Transform cameraTransform;
    cameraTransform.lookAtPos(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, -1.0f));

    kaun::Shader shader("media/shaders/default.frag", "media/shaders/default.vert");

    //kaun::Mesh* mesh = kaun::Mesh::box(1.0f, 1.0f, 1.0f, kaun::defaultVertexFormat);
    //kaun::Mesh* mesh = kaun::Mesh::sphere(1.0f, 32, 12, false, kaun::defaultVertexFormat);
    kaun::Mesh* mesh = kaun::Mesh::objFile("media/teapot.obj", kaun::defaultVertexFormat);
    mesh->normalize(true);
    kaun::Transform meshTrafo;

    kaun::Texture tex = kaun::Texture("media/default.png");
    //kaun::Texture* tex = kaun::Texture::checkerBoard(16, 16, 2);
    //kaun::Texture* tex = kaun::Texture::pixel(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));

    // kaun::setRenderState(...) // default state is set from the start

    kaun::RenderState state;
    //state.setCullFaces(kaun::RenderState::FaceDirections::NONE);
    //state.setFrontFace(kaun::RenderState::FaceOrientation::CCW);


    bool quit = false;
    kaun::closeSignal.connect([&quit]() {quit = true;});
    float lastTime = kaun::getTime();
    while(!quit) {
        kaun::setRenderTarget();
        kaun::clear(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        kaun::clearDepth();

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
