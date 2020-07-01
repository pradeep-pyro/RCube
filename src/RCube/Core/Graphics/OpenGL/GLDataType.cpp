#include "RCube/Core/Graphics/OpenGL/GLDataType.h"

namespace rcube
{
GLDataType getGLDataType(GLenum gl_enum)
{
    switch (gl_enum)
    {
    case GL_INT_VEC2:
        return GLDataType::Vec2i;
    case GL_INT_VEC3:
        return GLDataType::Vec3i;
    case GL_INT_VEC4:
        return GLDataType::Vec4i;
    case GL_FLOAT_VEC2:
        return GLDataType::Vec2f;
    case GL_FLOAT_VEC3:
        return GLDataType::Vec3f;
    case GL_FLOAT_VEC4:
        return GLDataType::Vec4f;
    case GL_FLOAT_MAT2:
        return GLDataType::Mat2f;
    case GL_FLOAT_MAT3:
        return GLDataType::Mat3f;
    case GL_FLOAT_MAT4:
        return GLDataType::Mat4f;
    case GL_FLOAT:
        return GLDataType::Float;
    case GL_INT:
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
        return GLDataType::Int;
    case GL_UNSIGNED_INT:
        return GLDataType::Uint;
    case GL_BOOL:
        return GLDataType::Bool;
    default:
        throw std::runtime_error("Unknown type: " + std::to_string(gl_enum));
    }
}

} // namespace rcube