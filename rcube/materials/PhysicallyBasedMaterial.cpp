#include "PhysicallyBasedMaterial.h"

int PhysicallyBasedMaterial::renderPriority() const {
    return RenderPriority::Opaque;
}
