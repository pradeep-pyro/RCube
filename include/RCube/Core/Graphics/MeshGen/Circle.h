#ifndef CIRCLE_H
#define CIRCLE_H

#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "glm/gtc/constants.hpp"

namespace rcube
{

/**
 * Creates mesh data representing a solid 2D disk
 * @param radius Radius of the disk
 * @param radial_segments NUmber of segments along the radius
 * @param theta_start Starting angle of the disk (default: 0)
 * @param theta_end Ending angle of the disk (default: 2pi)
 * @return A disk shaped mesh
 */
MeshData circle(float radius = 1, int radial_segments = 10, float theta_start = 0,
                float theta_end = glm::two_pi<float>());

} // namespace rcube

#endif // CIRCLE_H
