#pragma once

#include "glm/glm.hpp"
#include <memory>

#include "RCube/Core/Arch/Component.h"
#include "RCube/Core/Graphics/Materials/Material.h"
#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"
namespace rcube
{

/**
 * Drawable component represents 3D objects that can be drawn.
 * It is simply a collection of a Mesh representing the geometry, and a Material
 * representing the look of the object.
 * To create a valid object that will be rendered, add a Drawable component (object's apperance)
 * and a Transform component (object's location) to an Entity.
 */
class Drawable : public Component<Drawable>
{
  public:
    std::shared_ptr<Mesh> mesh;         /// OpenGL mesh
    std::shared_ptr<Material> material; /// Material describing the rendering appearance
    bool visible = true;                /// Whether visible when rendered

    void drawGUI();
};

} // namespace rcube
