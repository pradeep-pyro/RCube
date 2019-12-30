#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "glm/glm.hpp"
#include <memory>

#include "RCube/Core/Arch/Component.h"
#include "RCube/Core/Graphics/OpenGL/Material.h"
#include "RCube/Core/Graphics/OpenGL/Mesh.h"
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
    std::shared_ptr<ShaderProgram> material; /// Material describing the rendering appearance
    bool visible = true;                /// Whether visible when rendered
};

} // namespace rcube

#endif // DRAWABLE_H
