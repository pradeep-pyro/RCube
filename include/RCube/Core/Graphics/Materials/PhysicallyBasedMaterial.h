#ifndef PHYSICALLYBASEDMATERIAL_H
#define PHYSICALLYBASEDMATERIAL_H

#include "RCube/Core/Graphics/Materials/constants.h"
#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"
#include "glm/glm.hpp"
#include <string>

namespace rcube
{

/**
 * PhysicallyBasedMaterial is for displaying realistic surfaces with varying roughness and metalness
 * characteristics. It uses a physically-based Cook-Torrance BRDF for computing specular.
 */
std::shared_ptr<ShaderProgram> makePhysicallyBasedMaterial(glm::vec3 albedo = glm::vec3(1.0),
                                                           float roughness = 0.5f,
                                                           float metalness = 0.5f,
                                                           bool wireframe = false);

} // namespace rcube

#endif // PHYSICALLYBASEDMATERIAL_H
