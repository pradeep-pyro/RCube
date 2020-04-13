#ifndef OBJ_H
#define OBJ_H

#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include <string>

namespace rcube
{
/**
 * Creates mesh data from OBJ file
 * @param file_name file name of the input mesh
 */
TriangleMeshData loadOBJ(const std::string &file_name);
} // namespace rcube

#endif // OBJ_H