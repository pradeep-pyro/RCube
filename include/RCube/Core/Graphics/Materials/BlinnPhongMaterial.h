#ifndef BLINNPHONGMATERIAL_H
#define BLINNPHONGMATERIAL_H

#include "RCube/Core/Graphics/OpenGL/Material.h"
#include "RCube/Core/Graphics/OpenGL/Texture.h"
#include "constants.h"
#include "glm/glm.hpp"
#include <string>

namespace rcube
{

/**
 * BlinnPhongMaterial is for displaying shiny surfaces with specular highlights.
 * It uses a Blinn-Phong per-pixel shading model which is not physically-based.
 */
std::shared_ptr<ShaderProgram> makeBlinnPhongMaterial(glm::vec3 diffuse_color = glm::vec3(1.0),
                                                      glm::vec3 specular_color = glm::vec3(0.5f),
                                                      float shininess = 4.f,
                                                      bool wireframe = false);

} // namespace rcube

#endif // BLINNPHONGMATERIAL_H
