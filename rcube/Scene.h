#ifndef SCENE_H
#define SCENE_H

#include "ecs/world.h"

namespace rcube {

/**
 * Scene is the primary interface for the user and is a lightweight wrapper around
 * rube::World. It includes convenience methods for creating commonly used entities.
 * Additionally, this adds all available systems for processing the various components.
 *
 * rcube::World can also be used directly instead of rcube::Scene
 */
class Scene : public World {
public:
    Scene();
    EntityHandle createCamera();
    EntityHandle createDrawable();
    EntityHandle createDirectionalLight();
    EntityHandle createPointLight();
};

} // namespace rcube

#endif // SCENE_H
