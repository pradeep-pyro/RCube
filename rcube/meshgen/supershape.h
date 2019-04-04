#ifndef SUPERSHAPE_H
#define SUPERSHAPE_H

#include "../render/Mesh.h"

namespace rcube {

/**
 * Creates a super shape
 * @param radius Maximum radius of the supershape
 * @param latitude_segments
 * @param longitude_segments
 * @param a Real number (!=0)
 * @param b Real number (!=0)
 * @param m1 Real number
 * @param n1 Real number
 * @param n2 Real number
 * @param n3 Real number
 * @return Mesh of a supershape
 */
MeshData superShape(float radius, unsigned int parallels, unsigned int meridians,
                    float a, float b, float m1, float m2, float n1, float n2, float n3);

} // namespace rcube

#endif // SUPERSHAPE_H
