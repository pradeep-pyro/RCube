#include "cylinder.h"
#include "glm/gtc/constants.hpp"
#include "glm/gtx/string_cast.hpp"
#include "circle.h"

MeshData cylinder(float radius_bottom, float radius_top, float height, int radial_segments, int height_segments,
                  float theta_start, float theta_end, bool top_cap, bool bottom_cap) {
    MeshData data;
    data.indexed = true;
    data.primitive = MeshPrimitive::Triangles;

    float half_height = height / 2.f;
    float theta_inc = (theta_end - theta_start) / float(radial_segments);
    float height_inc = height / float(height_segments);
    float slope = (radius_bottom - radius_top) / height;

    for (int h = 0; h <= height_segments; ++h) {
        float curr_height = -half_height + h * height_inc;
        float h_ratio = float(h) / float(height_segments);
        float curr_radius = (1 - h_ratio) * radius_bottom + h_ratio * radius_top;
        for (int r = 0; r <= radial_segments; ++r) {
            float curr_theta = theta_start + r * theta_inc;
            float cos_theta = std::cos(curr_theta);
            float sin_theta = std::sin(curr_theta);
            data.vertices.push_back(glm::vec3(curr_radius * cos_theta, curr_height, curr_radius * sin_theta));
            data.normals.push_back(glm::normalize(glm::vec3(cos_theta, slope, sin_theta)));
        }
    }

    int stride = (radial_segments + 1);
    for (int i = 0; i < height_segments; i++) {
        for (int j = 0; j < radial_segments; j++) {
            int i_j = i * stride + j;
            int ip1_j = (i + 1) * stride + j;
            int i_jp1 = i * stride + j + 1;
            int ip1_jp1 = (i + 1) * stride + j + 1;
            data.indices.push_back(i_j);
            data.indices.push_back(i_jp1);
            data.indices.push_back(ip1_j);
            data.indices.push_back(ip1_j);
            data.indices.push_back(i_jp1);
            data.indices.push_back(ip1_jp1);
        }
    }

    if (top_cap) {
        MeshData top_cap = circle(radius_top, radial_segments, theta_start, theta_end);
        for (auto &v : top_cap.vertices) {
            v.y += half_height;
        }
        data.append(top_cap);
    }
    if (bottom_cap) {
        MeshData bottom_cap = circle(radius_bottom, radial_segments, theta_start, theta_end);
        for (auto &v : bottom_cap.vertices) {
            v.y -= half_height;
        }
        for (auto &n : bottom_cap.normals) {
            n.y *= -1;
        }
        data.append(bottom_cap);
    }

    return data;
}
