#pragma once

namespace rcube
{

typedef void (*procAddress)(const char *);

/**
 * Uses GLAD to initialize OpenGL as required by RCube
 * @param p OpenGL loader function (optional), e.g. glfwGetProcAddress
 */
void initGL(procAddress p = nullptr);

} // namespace rcube
