#ifndef RCUBE_H
#define RCUBE_H

#include <memory>

#include "RCube/Core/Arch/World.h"
#include "RCube/Systems/TransformSystem.h"
#include "RCube/Systems/CameraSystem.h"
#include "RCube/Systems/RenderSystem.h"

namespace rcube {

typedef void (*procAddress)(const char *);

/**
 * Uses GLAD to initialize OpenGL as required by RCube
 * @param p OpenGL loader function (optional), e.g. glfwGetProcAddress
 */
void initGL(procAddress p=nullptr);

} // namespace rcube

#endif // RCUBE_H
