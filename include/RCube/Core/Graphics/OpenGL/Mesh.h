#pragma once

#include "RCube/Core/Accel/BVH.h"
#include "RCube/Core/Graphics/OpenGL/AttributeBuffer.h"
#include "RCube/Core/Graphics/OpenGL/Buffer.h"
#include "RCube/Core/Graphics/OpenGL/GLDataType.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace rcube
{

enum class MeshPrimitive
{
    Points = GL_POINTS,
    Lines = GL_LINES,
    Triangles = GL_TRIANGLES
};

struct MeshData
{
    std::vector<glm::vec3> vertices, normals, colors, tangents;
    std::vector<glm::vec2> texcoords;
    std::vector<unsigned int> indices;
    MeshPrimitive primitive = MeshPrimitive::Triangles;
    bool indexed = false;

    void clear();

    void append(MeshData &other);

    bool valid() const;

    void scaleAndCenter();
};

// Represents a 3D triangle/line Mesh with vertex positions, normals,
// texcoords, colors using OpenGL buffers
class Mesh
{
    GLuint vao_ = 0;
    std::map<std::string, std::shared_ptr<AttributeBuffer>> attributes_;
    std::shared_ptr<AttributeIndexBuffer> indices_;
    bool init_ = false;
    BVHNodePtr bvh_;  // Bounding Volume Hierarchy for intersection queries

  public:
    MeshData data;

    Mesh() = default;

    Mesh(const Mesh &other) = delete;

    Mesh &operator=(const Mesh &other) = delete;

    /**
     * Initialize actually creates the vertex attribute object and buffers on the OpenGL side
     */
    static std::shared_ptr<Mesh> create(MeshPrimitive prim);

    static std::shared_ptr<Mesh> create(std::vector<std::shared_ptr<AttributeBuffer>> attributes,
                                        MeshPrimitive prim);

    void release();

    GLuint vao() const;

    bool valid() const;

    void use() const;

    void done() const;

    bool hasAttribute(std::string name) const;

    void uploadToGPU();

    size_t numVertexData() const;

    size_t numIndexData() const;

    void updateBVH();

    bool rayIntersect(const Ray &ray, glm::vec3 &pt, size_t &id);

    void enableAttribute(std::string name);

    void disableAttribute(std::string name);

  private:
    void setDefaultValue(GLuint id, const glm::vec3 &val);

    void setDefaultValue(GLuint id, const glm::vec2 &val);
};

} // namespace rcube
