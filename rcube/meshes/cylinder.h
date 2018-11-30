#ifndef CYLINDER_H
#define CYLINDER_H

#include "../render/Mesh.h"
#include "glm/gtc/constants.hpp"

MeshData cylinder(float radius_bottom=1, float radius_top=1, float height=1, int radial_segments=10, int height_segments=10,
                  float theta_start=0, float theta_end=glm::two_pi<float>(), bool top_cap=true, bool bottom_cap=true);

#endif // CYLINDER_H
