#ifndef GRID_H
#define GRID_H

#include "glm/glm.hpp"
#include "../render/Mesh.h"

MeshData grid(float width, float height, int width_segments, int height_segments,
              glm::vec3 color_centerline_x, glm::vec3 color_centerline_z,
              glm::vec3 color_grid);

#endif // GRID_H
