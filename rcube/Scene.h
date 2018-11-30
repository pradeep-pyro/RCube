#ifndef DEFAULTWORLD_H
#define DEFAULTWORLD_H

#include "ecs/world.h"

/**
 * Primary interface for the user which includes convenience methods for
 * creating commonly used entities. Additionally, this called adds all available systems
 * for processing various components.
 */

namespace rcube {

class Scene : public World {
public:
    Scene();
    EntityHandle createCamera();
    EntityHandle createDrawable();
    EntityHandle createDirectionalLight();
    EntityHandle createPointLight();
};

} // namespace rcube

#endif // DEFAULTWORLD_H
