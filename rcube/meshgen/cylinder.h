#ifndef CYLINDER_H
#define CYLINDER_H

#include "../render/Mesh.h"
#include "glm/gtc/constants.hpp"

/**
 * Creates a mesh representing a cylinder
 * @param radius_bottom Radius at the bottom of the cylinder
 * @param radius_top Radius at the top of the cylinder
 * @param height Height of the cylinder
 * @param radial_segments Number of segments along the radius
 * @param height_segments Number of segments along the height direction
 * @param theta_start Starting angle for the cylinder
 * @param theta_end Ending angle for the cylinder
 * @param top_cap Whether to include a top cap
 * @param bottom_cap Whether to include a bottom cap
 * @return A cylinder mesh
 */
MeshData cylinder(float radius_bottom=1, float radius_top=1, float height=1, int radial_segments=10, int height_segments=10,
                  float theta_start=0, float theta_end=glm::two_pi<float>(), bool top_cap=true, bool bottom_cap=true);

#endif // CYLINDER_H
