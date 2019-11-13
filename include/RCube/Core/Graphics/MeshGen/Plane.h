#ifndef PLANE_H
#define PLANE_H

#include <vector>
#include <memory>
#include "glm/glm.hpp"
#include "RCube/Core/Graphics/OpenGL/Mesh.h"

namespace rcube {

enum class Orientation {
    PositiveX, NegativeX,
    PositiveY, NegativeY,
    PositiveZ, NegativeZ,
};

/**
 * Creates a plane using triangle elements
 * @param width Width of the plane
 * @param height Height of the plane
 * @param width_segments Number of segments along the plane's width direction
 * @param height_segments Number of segments along the plane's height direction
 * @param ort Orientation of the plane: +X, -X, +Y, -Y, +Z, -Z
 * @return MeshData
 */
MeshData plane(float width, float height, unsigned int width_segments, unsigned int height_segments, Orientation ort);

} // namespace rcube

#endif // PLANE_H
