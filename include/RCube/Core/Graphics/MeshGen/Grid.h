#ifndef GRID_H
#define GRID_H

#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "glm/glm.hpp"

namespace rcube
{

/**
 * Create a planar grid (suitable as a ground plane in mesh viewers)
 * @param width Width of the grid
 * @param height Height of the grid
 * @param width_segments Number of segments along the width direction
 * @param height_segments Number of segments along the height direction
 * @param color_centerline_x Color of the centerline along x direction
 * @param color_centerline_z Color of the centerline along z direction
 * @param color_grid Color of the gridlines (except the centerlines)
 * @return A grid mesh
 */
MeshData grid(float width, float height, int width_segments, int height_segments,
              glm::vec3 color_centerline_x, glm::vec3 color_centerline_z, glm::vec3 color_grid);

} // namespace rcube

#endif // GRID_H
