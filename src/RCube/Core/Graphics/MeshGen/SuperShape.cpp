#include <iostream>
using namespace std;
#include "RCube/Core/Graphics/MeshGen/SuperShape.h"
#include "RCube/Core/Graphics/MeshGen/Sphere.h"
#include "glm/gtc/constants.hpp"

namespace rcube {

float superFormula(float angle, float a, float b, float m1, float m2, float n1, float n2, float n3) {
    using std::pow;
    using std::cos;
    using std::sin;
    using std::abs;
    float tmp = pow(abs(cos(m1 * angle * 0.25f) / a), n2) + pow(abs(sin(m2 * angle * 0.25f) / b), n3);
    return std::pow(tmp, -1.f / n1);
}

glm::vec3 superSphericalCoordinates(float lat, float lon, float lat_a, float lat_b, float lat_m, float lat_n1, float lat_n2, float lat_n3,
                                    float lon_a, float lon_b, float lon_m, float lon_n1, float lon_n2, float lon_n3) {
    float r1 = superFormula(lon, lon_a, lon_b, lon_m, lon_m, lon_n1, lon_n2, lon_n3);
    float r2 = superFormula(lat, lat_a, lat_b, lat_m, lat_m, lat_n1, lat_n2, lat_n3);
    float x = r1 * std::cos(lon) * r2 * std::cos(lat);
    float y = r1 * std::sin(lon) * r2 * std::cos(lat);
    float z = r2 * std::sin(lat);
    return glm::vec3(x, z, y);
}

MeshData superShape(float radius, unsigned int rows, unsigned int cols,
                    float a, float b, float m1, float m2, float n1, float n2, float n3) {
    MeshData data;
    data.indexed = true;
    data.primitive = MeshPrimitive::Triangles;
    float lat_inc = glm::pi<float>() / (rows - 1);
    float lon_inc = glm::two_pi<float>() / cols;
    data.vertices.reserve(rows * cols + 2);
    float max_dist = -100.f;

    const float pi = glm::pi<float>();
    const float half_pi = glm::half_pi<float>();
    for (unsigned int i = 0; i < rows - 2; ++i) {
        float lat = -half_pi + (i + 1) * lat_inc;
        for (unsigned int j = 0; j < cols; ++j) {
            float lon = -pi + j * lon_inc;
            glm::vec3 pt = superSphericalCoordinates(lat, lon,
                                                     a, b, m1, n1, n2, n3,
                                                     a, b, m2, n1, n2, n3);
            float new_length = glm::length(pt);
            if (max_dist < new_length) {
                max_dist = new_length;
            }
            data.vertices.push_back(pt);
        }
    }
    data.vertices.push_back(glm::vec3(0, 1, 0));
    data.vertices.push_back(glm::vec3(0, -1, 0));

    for (auto &v : data.vertices) {
        v /= max_dist;
        v *= radius;
    }

    data.indices.reserve(rows * cols + 2);


    for (unsigned int i = 0; i < rows - 3; ++i) {
        unsigned int r1 = i * cols;
        unsigned int r2 = (i + 1) * cols;
        for (unsigned int j = 0; j < cols; ++j) {
            data.indices.push_back(r1 + j);
            data.indices.push_back(r2 + j);
            data.indices.push_back(r2 + (j + 1) % cols);
            data.indices.push_back(r1 + j);
            data.indices.push_back(r2 + (j + 1) % cols);
            data.indices.push_back(r1 + (j + 1) % cols);
        }
    }

    unsigned int npole_idx = data.vertices.size() - 2;
    unsigned int spole_idx = data.vertices.size() - 1;
    for (unsigned int j = 0; j < cols; ++j) {
        // bottom cap
        data.indices.push_back(spole_idx);
        data.indices.push_back(1 * cols + (j + 1) % cols);
        data.indices.push_back(1 * cols + j);
        // top cap
        data.indices.push_back(npole_idx);
        data.indices.push_back((rows - 3) * cols + (j + 1) % cols);
        data.indices.push_back((rows - 3) * cols + j);
    }

    data.normals.resize(data.vertices.size(), glm::vec3(0));
    for (size_t i = 0; i < data.indices.size(); i += 3) {
        auto v1 = data.vertices[data.indices[i]];
        auto v2 = data.vertices[data.indices[i + 1]];
        auto v3 = data.vertices[data.indices[i + 2]];
        glm::vec3 fnormal = glm::normalize(glm::cross(v2 - v1, v3 - v1));
        data.normals[data.indices[i]] += fnormal;
        data.normals[data.indices[i + 1]] += fnormal;
        data.normals[data.indices[i + 2]] += fnormal;
    }

    for (auto &n : data.normals) {
        n = glm::normalize(n);
    }

    return data;
}

} // namespace rcube
