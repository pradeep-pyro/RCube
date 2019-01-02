#ifndef SUPERFORMULA_H
#define SUPERFORMULA_H

#include <memory>
#include "../render/Mesh.h"

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
 */
MeshData superShape(unsigned int latitude_segments, unsigned int longitude_segments,
                    float a, float b, float m, float n1, float n2, float n3);


#endif // SUPERFORMULA_H
