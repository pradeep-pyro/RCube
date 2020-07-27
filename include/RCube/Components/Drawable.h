#pragma once

#include "glm/glm.hpp"
#include <memory>

#include "RCube/Core/Arch/Component.h"
#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"
namespace rcube
{

/**
 * Drawable component represents 3D meshes that can be drawn.
 * It is simply a collection of a Mesh representing the geometry,
 * and some associated properties..
 *
 * To create a valid object that can be rendered, add a Mesh,
 * Transform and a Material to an Entity.
 */
class Drawable : public Component<Drawable>
{
  public:
    std::shared_ptr<Mesh> mesh; /// OpenGL mesh
    bool visible = true;        /// Whether visible when rendered

    void drawGUI();
};

} // namespace rcube
