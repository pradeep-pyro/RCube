#ifndef BOX_H
#define BOX_H

#include <memory>
#include "../render/Mesh.h"

MeshData box(float width, float height, float depth, unsigned int width_segments,
             unsigned int height_segments, unsigned int depth_segments);
#endif // BOX_H
