#include "RCube/RCube.h"
#include "glad/glad.h"
#include <stdexcept>

namespace rcube
{

void initGL(procAddress p)
{
    int ok = (p == nullptr) ? gladLoadGL() : gladLoadGLLoader((GLADloadproc)p);
    if (!ok)
    {
        throw std::runtime_error("Failed to initialize OpenGL context");
    }

    if (GLVersion.major != 4 || GLVersion.minor < 5)
    {
        throw std::runtime_error("RCube requires OpenGL 4.5");
    }
}

} // namespace rcube
