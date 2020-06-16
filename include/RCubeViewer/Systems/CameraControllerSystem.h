#pragma once

#include "RCube/Components/Camera.h"
#include "RCube/Components/Transform.h"
#include "RCube/Core/Arch/System.h"
#include "RCube/Core/Arch/World.h"
#include "RCube/Window.h"
#include "RCubeViewer/Components/CameraController.h"

namespace rcube
{
namespace viewer
{

class CameraControllerSystem : public System
{
  public:
    CameraControllerSystem();
    virtual void update(bool force) override;

    virtual const std::string name() const override;

    unsigned int priority() const override;
};

} // namespace viewer
} // namespace rcube
