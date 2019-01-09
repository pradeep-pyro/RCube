#ifndef SPHERE_H
#define SPHERE_H

#include "../render/Mesh.h"

namespace rcube {

MeshData icoSphere(float radius=1, unsigned int subdivisions=0);

MeshData cubeSphere(float radius=1, unsigned int n_segments=1);

MeshData latlonSphere(float radius=1, int lat_segments=5, int lon_segments=5);

} // namespace rcube

#endif // SPHERE_H
