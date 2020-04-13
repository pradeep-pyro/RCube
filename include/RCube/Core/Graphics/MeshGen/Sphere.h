#ifndef SPHERE_H
#define SPHERE_H

#include "RCube/Core/Graphics/OpenGL/Mesh.h"
namespace rcube
{

TriangleMeshData icoSphere(float radius = 1, unsigned int subdivisions = 0);

TriangleMeshData cubeSphere(float radius = 1, unsigned int n_segments = 1);

} // namespace rcube

#endif // SPHERE_H
