#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <memory>
#include "glm/glm.hpp"

#include "../render/Mesh.h"
#include "../render/Material.h"
#include "../ecs/component.h"

namespace rcube {

/**
 * Drawable component represents 3D objects that can be drawn.
 * It is simply a collection of a Mesh representing the geometry, and a Material
 * representing the look of the object.
 * To create a valid object that will be rendered, add a Drawable component (object's apperance)
 * and a Transform component (object's location) to an Entity.
 */
class Drawable : public Component<Drawable> {
public:
    Drawable();
    Drawable(const Drawable &other) = default;
    Drawable & operator=(const Drawable &other) = default;
    Drawable(Drawable &&other) = default;
    virtual ~Drawable() = default;

    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    bool visible = true;
};

} // namespace rcube

#endif // DRAWABLE_H
