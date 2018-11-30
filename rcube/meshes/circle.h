#ifndef CIRCLE_H
#define CIRCLE_H

#include "../render/Mesh.h"
#include "glm/gtc/constants.hpp"

MeshData circle(float radius=1, int radial_segments=10, float theta_start=0, float theta_end=glm::two_pi<float>());

#endif // CIRCLE_H
