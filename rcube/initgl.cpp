#include "initgl.h"

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

} // namespace rcube
