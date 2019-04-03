#include <iostream>
using namespace std;
#include "supershape.h"
#include "sphere.h"
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
    return glm::vec3(x, y, z);
}

MeshData superShape(float radius, unsigned int latitude_segments, unsigned int longitude_segments,
                    float a, float b, float m1, float m2, float n1, float n2, float n3) {
    MeshData data;
    data.indexed = true;
    data.primitive = MeshPrimitive::Triangles;
    float lat_inc = glm::pi<float>() / (latitude_segments);
    float lon_inc = glm::two_pi<float>() / (longitude_segments - 1);
    data.vertices.reserve(latitude_segments * longitude_segments - 1);
    float max_dist = -100.f;
    for (int i = 0; i < latitude_segments; ++i) {
        float lat = -glm::half_pi<float>() + i * lat_inc;
        for (int j = 0; j < longitude_segments; ++j) {
            float lon = -glm::pi<float>() + j * lon_inc;
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
    for (auto &v : data.vertices) {
        v /= max_dist;
        v *= radius;
    }

    // Split each grid square into 2 triangles and store indices in F
    for (unsigned int i = 0; i < latitude_segments - 1; ++i) {
        for (unsigned int j = 0; j < longitude_segments - 1; ++j) {
            data.indices.push_back(i*longitude_segments + j);
            data.indices.push_back(i*longitude_segments + j + 1);
            data.indices.push_back((i + 1) * longitude_segments + j + 1);
            data.indices.push_back(i * longitude_segments + j);
            data.indices.push_back((i + 1) * longitude_segments + j + 1);
            data.indices.push_back((i + 1) * longitude_segments + j);
        }
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
