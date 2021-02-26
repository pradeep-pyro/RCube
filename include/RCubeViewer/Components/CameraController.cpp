#include "CameraController.h"
#include "imgui.h"

void rcube::viewer::CameraController::drawGUI()
{
    ImGui::SliderFloat("Rotation speed", &rotate_speed, 1, 10);
    ImGui::SliderFloat("Pan speed", &pan_speed, 1, 10);
    ImGui::SliderFloat("Zoom speed", &zoom_speed, 0.5f, 10);
    ImGui::Separator();
    ImGui::SliderAngle("Min. horizonal angle", &min_horizontal_angle);
    ImGui::SliderAngle("Max. horizonal angle", &max_horizontal_angle);
    ImGui::SliderAngle("Min. vertical angle", &min_vertical_angle, -glm::pi<float>(),
                       glm::pi<float>());
    ImGui::SliderAngle("Max. vertical angle", &max_vertical_angle);
}
