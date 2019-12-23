#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"

#include "glad/glad.h"
#include <iostream>

namespace rcube
{

void _checkGLError(const char *file, int line)
{
    GLenum err = glGetError();

    while (err != GL_NO_ERROR)
    {
        std::string error;
        if (err == GL_INVALID_OPERATION)
        {
            error = "GL_INVALID_OPERATION";
        }
        else if (err == GL_INVALID_ENUM)
        {
            error = "GL_INVALID_ENUM";
        }
        else if (err == GL_INVALID_VALUE)
        {
            error = "GL_INVALID_VALUE";
        }
        else if (err == GL_OUT_OF_MEMORY)
        {
            error = "GL_OUT_OF_MEMORY";
        }
        else if (err == GL_INVALID_FRAMEBUFFER_OPERATION)
        {
            error = "GL_INVALID_FRAMEBUFFER_OPERATION";
        }

        std::cerr << error.c_str() << " - " << file << ":" << line << std::endl;
        err = glGetError();
    }
}

} // namespace rcube
