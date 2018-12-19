#ifndef SUPERFORMULA_H
#define SUPERFORMULA_H

#include <memory>
#include "../render/Mesh.h"

MeshData superShape(float scale, unsigned int latitude_segments, unsigned int longitude_segments,
                    float a, float b, float m, float n1, float n2, float n3);

/**
 *
 * @param scale Scale of the mesh
 * @param latitude_segments Number of segments along the latitude/polar direction
 * @param longitude_segments Number of segments along the longitude/azimuth direction
 * @param lat_a Frequency along latitude (corresponds to bumps)
 * @param lat_b
 * @param lat_m
 * @param lat_n1
 * @param lat_n2
 * @param lat_n3
 * @param lon_a
 * @param lon_b
 * @param lon_m
 * @param lon_n1
 * @param lon_n2
 * @param lon_n3
 * @return
 */
MeshData superShape(float scale, unsigned int latitude_segments, unsigned int longitude_segments,
                    float lat_a, float lat_b, float lat_m, float lat_n1, float lat_n2, float lat_n3,
                    float lon_a, float lon_b, float lon_m, float lon_n1, float lon_n2, float lon_n3);

#endif // SUPERFORMULA_H
