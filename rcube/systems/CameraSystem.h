#ifndef CAMERASYSTEM_H
#define CAMERASYSTEM_H

#include "../ecs/system.h"

namespace rcube {

class CameraSystem : public System {
public:
    CameraSystem();
    virtual void initialize() override {}
    virtual void cleanup() override {}
    virtual void update(bool force) override;
    virtual unsigned int priority() const override;
};

} // namespace rcube

#endif // CAMERASYSTEM_H
