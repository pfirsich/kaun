#include <kaun.hpp>
#include <glm/glm.hpp>

void resize(int w, int h) {
    kaun::setProjection(glm::perspective(glm::radians(45.0f), static_cast<float>(w)/h, 0.1f, 100.0f));
    kaun::setViewport(0, 0, w, h);
}

int main(int argc, char** argv) {
    kaun::WindowProperties props;
    props.msaaSamples = 8;
    props.srgb = true;
    kaun::setupWindow("Kaun Test", 1600, 900, props);
    kaun::init();

    kaun::resizeSignal.connect(resize);
    kaun::resizeSignal.emit(kaun::getWindowWidth(), kaun::getWindowHeight());

    kaun::Transform cameraTransform;
    cameraTransform.lookAtPos(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, -1.0f));

    kaun::Shader shader("media/shaders/default.frag", "media/shaders/default.vert");

    kaun::Mesh* planeMesh = kaun::Mesh::plane(2.0f, 2.0f, 1, 1, kaun::defaultVertexFormat);
    kaun::Transform planeTrafo;
    planeTrafo.rotateWorld(-glm::half_pi<float>()*0.5f, glm::vec3(1.0f, 0.0f, 0.0f));

    //kaun::Mesh* mesh = kaun::Mesh::box(1.0f, 1.0f, 1.0f, kaun::defaultVertexFormat);
    kaun::Mesh* mesh = kaun::Mesh::plane(1.0f, 1.0f, 1, 1, kaun::defaultVertexFormat);
    //kaun::Mesh* mesh = kaun::Mesh::sphere(1.0f, 32, 12, false, kaun::defaultVertexFormat);
    //kaun::Mesh* mesh = kaun::Mesh::objFile("media/bunny.obj", kaun::defaultVertexFormat);
    //mesh->normalize(true);
    kaun::Transform meshTrafo;
    meshTrafo.rotateWorld(-glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));

    //kaun::Texture tex = kaun::Texture("media/default.png");
    //kaun::Texture* tex = kaun::Texture::checkerBoard(16, 16, 2);
    //kaun::Texture* tex = kaun::Texture::pixel(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));

    //kaun::defaultRenderState.setCullFaces(kaun::RenderState::FaceDirections::NONE);

    kaun::setViewTransform(cameraTransform);

    auto* colorTarget = new kaun::Texture(kaun::PixelFormat::SRGB8A8, 1600, 900);
    auto* depthTarget = new kaun::Texture(kaun::PixelFormat::DEPTH24, 1600, 900);

    bool quit = false;
    kaun::closeSignal.connect([&quit]() {quit = true;});
    float lastTime = kaun::getTime();
    while(!quit) {
        // update
        float t = kaun::getTime();
        float dt = t - lastTime;
        lastTime = t;

        //meshTrafo.rotateWorld(dt, glm::vec3(0.0f, 1.0f, 0.0f));

        // draw
        kaun::setRenderTarget({colorTarget}, depthTarget);
        kaun::clear(glm::vec4(0.9f, 1.0f, 1.0f, 1.0f));
        kaun::clearDepth();

        kaun::setModelTransform(meshTrafo);
        kaun::draw(*mesh, shader, {
            kaun::Uniform("color", glm::vec3(1.0f, 0.0f, 0.0f))
        });

        kaun::setRenderTarget({}, nullptr, true);

        kaun::setModelTransform(planeTrafo);
        kaun::draw(*planeMesh, shader, {
            kaun::Uniform("color", glm::vec3(0.0f, 1.0f, 0.0f))
        });

        kaun::flush();

        kaun::updateAndSwap();
    }
    kaun::cleanup();

    return 0;
}
