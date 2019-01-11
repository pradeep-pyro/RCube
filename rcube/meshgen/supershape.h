#ifndef SUPERSHAPE_H
#define SUPERSHAPE_H

#include <memory>
#include "../render/Mesh.h"

namespace rcube {

/**
 * Creates a super shape
 * @param latitude_segments
 * @param longitude_segments
 * @param a Real number (!=0)
 * @param b Real number (!=0)
 * @param m Real number
 * @param n1 Real number
 * @param n2 Real number
 * @param n3 Real number
 * @return Mesh of a supershape
 * TODO: use icosahedron for initial spherical mesh
 */
MeshData superShape(unsigned int latitude_segments, unsigned int longitude_segments,
                    float a, float b, float m, float n1, float n2, float n3);

} // namespace rcube

#endif // SUPERSHAPE_H
