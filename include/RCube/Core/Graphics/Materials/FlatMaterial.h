#ifndef FLATMATERIAL_H
#define FLATMATERIAL_H

#include "RCube/Core/Graphics/OpenGL/Material.h"
#include "constants.h"

namespace rcube
{

/**
 * FlatMaterial is for representing objects in a flat style without any 3D shading
 */
std::shared_ptr<ShaderProgram> makeFlatMaterial();

} // namespace rcube

#endif // FLATMATERIAL_H
