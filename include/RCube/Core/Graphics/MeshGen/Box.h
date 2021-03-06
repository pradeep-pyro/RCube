#ifndef BOX_H
#define BOX_H
#include "RCube/Core/Graphics/OpenGL/Mesh.h"

namespace rcube
{

/**
 * Creates mesh data representing a box.
 * Note that the texture coordinates parametrize each face of the box, resulting in a single texture
 * to be displayed in all of them
 * @param width Width of the box
 * @param height Height of the box
 * @param depth Depth of the box
 * @param width_segments Number of segments along the width axis
 * @param height_segments Number of segments along the height axis
 * @param depth_segments Number of segments along the depth axis
 * @return A box mesh
 */
TriangleMeshData box(float width, float height, float depth, unsigned int width_segments,
             unsigned int height_segments, unsigned int depth_segments);

} // namespace rcube

#endif // BOX_H
