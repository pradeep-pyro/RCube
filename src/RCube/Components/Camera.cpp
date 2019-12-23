#include "RCube/Components/Camera.h"

namespace rcube
{

Frustum Camera::frustum()
{
    static glm::vec4 cube[8] = {
        glm::vec4(-1, 1, -1, 1),  // near topleft
        glm::vec4(1, 1, -1, 1),   // near topright
        glm::vec4(-1, -1, -1, 1), // near bottomleft
        glm::vec4(-1, 1, -1, 1),  // near bottomright
        glm::vec4(-1, 1, 1, 1),   // far topleft
        glm::vec4(1, 1, 1, 1),    // far topright
        glm::vec4(-1, -1, 1, 1),  // far bottomleft
        glm::vec4(-1, 1, 1, 1)    // far bottomright
    };

    glm::mat4 invVP = glm::inverse(world_to_view * view_to_projection);
    Frustum fr;
    for (size_t i = 0; i < 8; ++i)
    {
        glm::vec4 tmp = invVP * cube[i];
        tmp /= tmp.w;
        fr.points[i].x = tmp.x;
        fr.points[i].y = tmp.y;
        fr.points[i].z = tmp.z;
    }
    return fr;
}

} // namespace rcube
