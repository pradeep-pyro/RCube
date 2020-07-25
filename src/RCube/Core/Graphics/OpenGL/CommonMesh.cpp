#include "RCube/Core/Graphics/OpenGL/CommonMesh.h"

namespace rcube
{
namespace common
{

std::shared_ptr<Mesh> fullScreenQuadMesh()
{
    auto mesh = Mesh::create({AttributeBuffer::create("positions", AttributeLocation::POSITION, 3),
                              AttributeBuffer::create("uvs", AttributeLocation::UV, 2)},
                             MeshPrimitive::Triangles);
    mesh->attribute("positions")
        ->setData(std::vector<glm::vec3>{glm::vec3(-1.f, -1.f, 0.f), glm::vec3(3.f, -1.f, 0.f),
                                         glm::vec3(-1.f, 3.f, 0.f)});
    mesh->attribute("uvs")->setData(
        std::vector<glm::vec2>{glm::vec2(0, 0), glm::vec2(2, 0), glm::vec2(0, 2)});
    mesh->uploadToGPU();
    return mesh;
}

std::shared_ptr<Mesh> skyboxMesh()
{
    auto mesh = Mesh::create({AttributeBuffer::create("positions", AttributeLocation::POSITION, 3)},
                             MeshPrimitive::Triangles);
    mesh->attribute("positions")
        ->setData(std::vector<glm::vec3>{
                   glm::vec3(-1.0f, 1.0f, -1.0f),  glm::vec3(-1.0f, -1.0f, -1.0f),
                   glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(1.0f, -1.0f, -1.0f),
                   glm::vec3(1.0f, 1.0f, -1.0f),   glm::vec3(-1.0f, 1.0f, -1.0f),

                   glm::vec3(-1.0f, -1.0f, 1.0f),  glm::vec3(-1.0f, -1.0f, -1.0f),
                   glm::vec3(-1.0f, 1.0f, -1.0f),  glm::vec3(-1.0f, 1.0f, -1.0f),
                   glm::vec3(-1.0f, 1.0f, 1.0f),   glm::vec3(-1.0f, -1.0f, 1.0f),

                   glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(1.0f, -1.0f, 1.0f),
                   glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec3(1.0f, 1.0f, 1.0f),
                   glm::vec3(1.0f, 1.0f, -1.0f),   glm::vec3(1.0f, -1.0f, -1.0f),

                   glm::vec3(-1.0f, -1.0f, 1.0f),  glm::vec3(-1.0f, 1.0f, 1.0f),
                   glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec3(1.0f, 1.0f, 1.0f),
                   glm::vec3(1.0f, -1.0f, 1.0f),   glm::vec3(-1.0f, -1.0f, 1.0f),

                   glm::vec3(-1.0f, 1.0f, -1.0f),  glm::vec3(1.0f, 1.0f, -1.0f),
                   glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec3(1.0f, 1.0f, 1.0f),
                   glm::vec3(-1.0f, 1.0f, 1.0f),   glm::vec3(-1.0f, 1.0f, -1.0f),

                   glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, -1.0f, 1.0f),
                   glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(1.0f, -1.0f, -1.0f),
                   glm::vec3(-1.0f, -1.0f, 1.0f),  glm::vec3(1.0f, -1.0f, 1.0f)});
    mesh->uploadToGPU();
    return mesh;
}

} // namespace common
} // namespace rcube