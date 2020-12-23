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
    Triangles = GL_TRIANGLES,
    TriangleStrip = GL_TRIANGLE_STRIP
};

struct LineMeshData
{
    std::vector<glm::vec3> vertices, colors;
    std::vector<glm::uvec2> indices;
    bool indexed = false;

    void clear();

    void append(LineMeshData &other);

    bool valid() const;

    void scaleAndCenter();
};


struct TriangleMeshData
{
    std::vector<glm::vec3> vertices, normals, colors, tangents;
    std::vector<glm::vec2> texcoords;
    std::vector<glm::uvec3> indices;
    bool indexed = false;

    void clear();

    void append(TriangleMeshData &other);

    bool valid() const;

    void scaleAndCenter();
};

// Represents a 3D triangle/line Mesh with vertex positions, normals,
// texcoords, colors using OpenGL buffers
class Mesh
{
  protected:
    GLuint vao_ = 0;
    MeshPrimitive primitive_ = MeshPrimitive::Triangles;
    std::map<std::string, std::shared_ptr<AttributeBuffer>> attributes_;
    std::shared_ptr<AttributeIndexBuffer> indices_;
    std::map<std::string, bool> attributes_enabled_; 
    bool init_ = false;
    BVHNodePtr bvh_;  // Bounding Volume Hierarchy for intersection queries

    Mesh(std::vector<std::shared_ptr<AttributeBuffer>> attributes, MeshPrimitive prim,
         bool indexed = false);
  public:
    Mesh() = default;

    virtual ~Mesh() = default;

    Mesh(const Mesh &other) = delete;

    Mesh &operator=(const Mesh &other) = delete;

    /**
     * Initialize actually creates the vertex attribute object and buffers on the OpenGL side
     */

    static std::shared_ptr<Mesh> createPointMesh();

    static std::shared_ptr<Mesh> createLineMesh(bool indexed);

    static std::shared_ptr<Mesh> createTriangleMesh(bool indexed, bool strip = false);

    static std::shared_ptr<Mesh> create(const LineMeshData &linemesh);

    static std::shared_ptr<Mesh> create(const TriangleMeshData &trimesh);

    static std::shared_ptr<Mesh> create(std::vector<std::shared_ptr<AttributeBuffer>> attributes,
                                        MeshPrimitive prim, bool indexed=false);

    void release();

    GLuint vao() const;

    bool valid() const;

    void use() const;

    void done() const;

    bool hasAttribute(std::string name) const;

    std::shared_ptr<AttributeBuffer> attribute(std::string name);

    std::shared_ptr<AttributeIndexBuffer> indices();

    void uploadToGPU();

    MeshPrimitive primitive() const
    {
        return primitive_;
    }

    size_t primitiveDim() const
    {
        return primitive_ == MeshPrimitive::Points ? 1 : (primitive_ == MeshPrimitive::Lines ? 2 : 3);
    }

    const auto& attributes() const
    {
        return attributes_;
    }

    bool attributeEnabled(std::string name) const
    {
        return attributes_enabled_.at(name);
    }

    size_t numVertexData() const;

    size_t numIndexData() const;

    void updateBVH();

    bool rayIntersect(const Ray &ray, glm::vec3 &pt, size_t &id);

    void enableAttribute(std::string name);

    void disableAttribute(std::string name);

    virtual void drawGUI();

  private:
    void setDefaultValue(GLuint id, const glm::vec3 &val);

    void setDefaultValue(GLuint id, const glm::vec2 &val);
};

} // namespace rcube
