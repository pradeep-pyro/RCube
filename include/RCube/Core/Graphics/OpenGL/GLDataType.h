#pragma once

#include <string>
#include <stdexcept>
#include "glad/glad.h"

namespace rcube
{

enum class GLDataType
{
    Vec2i = GL_INT_VEC2,
    Vec3i = GL_INT_VEC3,
    Vec4i = GL_INT_VEC4,
    Vec2f = GL_FLOAT_VEC2,
    Vec3f = GL_FLOAT_VEC3,
    Vec4f = GL_FLOAT_VEC4,
    Mat2f = GL_FLOAT_MAT2,
    Mat3f = GL_FLOAT_MAT3,
    Mat4f = GL_FLOAT_MAT4,
    Float = GL_FLOAT,
    Int = GL_INT,
    Bool = GL_BOOL,
    Uint = GL_UNSIGNED_INT
};

GLDataType getGLDataType(GLenum gl_enum);

} // namespace rcube