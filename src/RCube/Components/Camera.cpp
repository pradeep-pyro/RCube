#include "RCube/Components/Camera.h"
#include "RCube/Core/Graphics/TexGen/Gradient.h"

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

glm::vec3 Camera::viewportToWorld(glm::vec2 xy, float distance_from_camera)
{
    glm::mat4 viewport_to_world = glm::inverse(projection_to_viewport * view_to_projection * world_to_view);
    return glm::vec3(viewport_to_world *glm::vec4(xy.x, xy.y, distance_from_camera, 1.0));
}

void Camera::createGradientSkyBox(const glm::vec3 &color_top, const glm::vec3 &color_bot)
{
    skybox = TextureCubemap::create(256, 256, 1, true, TextureInternalFormat::sRGB8);
    Image front_back = gradientV(256, 256, color_top, color_bot, 2.f);
    Image top = gradientV(256, 256, color_top, color_top, 2.f);
    Image bottom = gradientV(256, 256, color_bot, color_bot, 2.f);
    skybox->setFilterModeMin(rcube::TextureFilterMode::Trilinear);
    skybox->setData(TextureCubemap::PositiveY, top);
    skybox->setData(TextureCubemap::NegativeY, bottom);
    skybox->setData(TextureCubemap::PositiveX, front_back);
    skybox->setData(TextureCubemap::NegativeX, front_back);
    skybox->setData(TextureCubemap::NegativeZ, front_back);
    skybox->setData(TextureCubemap::PositiveZ, front_back);
}

void Camera::drawGUI()
{
    /*
    // TODO(pradeep): disabling as FOV 1 in perspective
    // gives something similar with skybox support
    ImGui::Checkbox("Orthographic", &orthographic);
    if (orthographic)
    {
        ImGui::InputFloat("Orthographic Width", &orthographic_size);
    }
    else
    {*/
    ImGui::SliderAngle("FOV (deg.)", &fov, 1.f, 89.f);
    //}
    ImGui::InputFloat("Near Plane", &near_plane);
    ImGui::InputFloat("Far Plane", &far_plane);
    ImGui::InputFloat("Bloom Threshold", &bloom_threshold);
}

} // namespace rcube
