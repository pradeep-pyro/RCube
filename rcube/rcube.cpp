#include "rcube.h"
#include "glad/glad.h"

namespace rcube {

void initGL(procAddress p) {
    int ok = (p == nullptr) ? gladLoadGL() : gladLoadGLLoader((GLADloadproc) p);
    if (!ok) {
        throw std::runtime_error("Failed to initialize OpenGL context");
    }

    if (GLVersion.major != 4 || GLVersion.minor < 2) {
        throw std::runtime_error("RCube requires OpenGL 4.2");
    }
}

World&& makeWorld() {
    World world;
    world.addSystem(std::make_unique<TransformSystem>());
    world.addSystem(std::make_unique<CameraSystem>());
    world.addSystem(std::make_unique<RenderSystem>());
    return std::move(world);
}

} // namespace rcube
