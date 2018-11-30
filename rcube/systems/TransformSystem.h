#ifndef TRANSFORMSYSTEM_H
#define TRANSFORMSYSTEM_H

#include "../ecs/system.h"

namespace rcube {

/**
 * TransformSystem is an ECS system to calculate the transformation matrix
 * (model-to-world) of every Transform component while considering the
 * Transform hierarchy.
 */
class TransformSystem : public System {
public:
    TransformSystem();
    virtual ~TransformSystem() override = default;
    virtual void initialize() override;
    virtual void cleanup() override;
    virtual void update(bool force=false) override;
};

} // namespace rcube

#endif // TRANSFORMSYSTEM_H
