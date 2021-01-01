#pragma once

#include "RCube/Core/Arch/System.h"

namespace rcube
{
namespace viewer
{
/**
 * The PickTooltipSystem is used to display a tooltip with scalar/vector field information when
 * elements in a Pointcloud or SurfaceMesh are picked.
 */
class PickTooltipSystem : public System
{
  public:
    PickTooltipSystem();
    virtual ~PickTooltipSystem() = default;
    virtual void update(bool) override;
    virtual unsigned int priority() const override;
    virtual const std::string name() const override;
};

} // namespace viewer
} // namespace rcube