#ifndef ARCBALL_H
#define ARCBALL_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace rcube {

// Arcball simulates an object rotation about the origin in model space
class Arcball {
public:
    explicit Arcball(int width=0, int height=0, float speed=1.0f) : speed_(speed), width_(width), height_(height), active_(false) {
        quatSetIdentity(last_rot_);
        quatSetIdentity(curr_rot_);
        last_pos_.x = 0;
        last_pos_.y = 0;
    }

    // Start rotation (called in the mouse press event)
    void start(int screen_x, int screen_y) {
        active_ = true;
        last_pos_.x = screen_x;
        last_pos_.y = screen_y;
        quatSetIdentity(curr_rot_);
    }

    // Stop rotation (called in the mouse release event)
    void stop(int screen_x, int screen_y) {
        active_ = false;
        last_pos_.x = screen_x;
        last_pos_.y = screen_y;
        last_rot_ = glm::normalize(curr_rot_ * last_rot_);
        quatSetIdentity(curr_rot_);
    }

    void reset(int screen_x, int screen_y) {
        last_pos_.x = screen_x;
        last_pos_.y = screen_y;
        quatSetIdentity(last_rot_);
        quatSetIdentity(curr_rot_);
    }

    // Compute and accumulate rotation in quaternion form (called in the mouse motion event)
    void rotate(int screen_x, int screen_y) {
        if (!active() || (last_pos_.x == screen_x && last_pos_.y == screen_y)) {
            return;
        }
        glm::vec3 P = arcballVector(last_pos_.x, last_pos_.y);
        glm::vec3 Q = arcballVector(screen_x, screen_y);
        glm::vec3 axis = glm::cross(P, Q);
        float cos_angle = glm::dot(P, Q);
        float sin_angle = std::sqrt(glm::dot(axis, axis));
        float angle = speed_ * std::atan2(sin_angle, cos_angle);
        curr_rot_ = glm::angleAxis(angle, glm::normalize(axis));
    }

    bool active() const {
        return active_;
    }

    float speed() const {
        return speed_;
    }

    void setSpeed(float speed) {
        speed_ = speed;
    }

    void resize(float width, float height) {
        width_ = width;
        height_ = height;
    }

    float width() const {
        return width_;
    }

    float height() const {
        return height_;
    }

    // Get the rotation matrix in view space to transform the scene
    glm::mat4 matrix() {
        return glm::toMat4(glm::normalize(curr_rot_ * last_rot_));
    }

private:
    float speed_, width_, height_;
    glm::quat last_rot_, curr_rot_;
    glm::ivec2 last_pos_;
    bool active_;

    glm::vec3 arcballVector(int x, int y) {
        glm::vec3 vec((2.f * x - width_) / width_, (2.f * y - height_) / height_, 0.f);
        vec.y = -vec.y;
        float sqlen = vec.x * vec.x + vec.y * vec.y;
        if (sqlen <= 1.0f) {
            vec.z = 1.0f - glm::sqrt(sqlen);
        }
        else {
            vec = glm::normalize(vec);
        }
        return vec;
    }

    void quatSetIdentity(glm::quat & q) {
        q.w = 1.0f;
        q.x = 0.0f;
        q.y = 0.0f;
        q.z = 0.0f;
    }
};

} // namespace rcube

#endif // ARCBALL_H
