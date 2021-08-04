#pragma once

#include "RCube/Core/Arch/System.h"

namespace rcube
{

class CameraSystem : public System
{
  public:
    CameraSystem();
    virtual void initialize() override
    {
    }
    virtual void cleanup() override
    {
    }
    virtual void update(bool force) override;
    virtual unsigned int priority() const override;
    virtual const std::string name() const override
    {
        return "CameraSystem";
    }
};

} // namespace rcube
