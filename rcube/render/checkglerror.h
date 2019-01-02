#ifndef CHECK_GL_ERROR_H
#define CHECK_GL_ERROR_H

void _checkGLError(const char *file, int line);

#define checkGLError() _checkGLError(__FILE__,__LINE__)

#endif // CHECK_GL_ERROR_H
