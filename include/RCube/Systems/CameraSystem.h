#pragma once

#include "RCube/Core/Arch/System.h"
#include "RCube/Core/Accel/AABB.h"
#include "RCube/Components/Camera.h"
#include "RCube/Components/Transform.h"

namespace rcube
{

class CameraSystem : public System
{
  public:
    CameraSystem();
    virtual void preUpdate() override;
    virtual void update(bool force) override;
    virtual void postUpdate() override;
    virtual unsigned int priority() const override;
    void fitToExtents(Camera *cam, Transform *tr, const AABB &scene_bounding_box_world_space);
    void fitNearFarPlanes(Camera *cam, const AABB &scene_bounding_box_world_space);
    virtual const std::string name() const override
    {
        return "CameraSystem";
    }
};

} // namespace rcube
